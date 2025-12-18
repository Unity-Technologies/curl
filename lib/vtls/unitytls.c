/*
 * Source file for all unitytls-specific code for the TLS/SSL layer. No code
 * but vtls.c should ever call or use these functions.
 *
 */

#include "curl_setup.h"

#ifdef USE_UNITYTLS

#include "unitytls_interface.h"
#include "urldata.h"
#include "sendf.h"
#include "vtls.h"
#include "vtls_int.h"
#include "connect.h" /* for the connect timeout */
#include "select.h"
#include "curl_printf.h"
#include "curlx/multibyte.h"

#if !defined(WIN32)
#include <dirent.h>
#endif

/* The last #include files should be: */
#include "curl_memory.h"
#include "memdebug.h"

/* struct for data related to each SSL connection */
struct ssl_backend_data {
  unitytls_x509list* cacert;
  unitytls_x509list* clicert;
  unitytls_key* pk;
  unitytls_tlsctx* ctx;
#ifdef HAS_ALPN
  const char *protocols[ALPN_ENTRIES_MAX + 1];
#endif
};

/*
* UnityTls interface
*
* Usually we expect the user to call curl_unitytls_set_interface before using curl.
* However, this does not work for running curls' tests in which case we define UNITYTLS_LINKED and link directly against the UnityTLS module
*/
//#define UNITYTLS_LINKED

static unitytls_interface_struct* unitytls = NULL;

#if defined(UNITYTLS_LINKED)
extern unitytls_interface_struct* unitytls_get_interface_struct();
#endif

void curl_unitytls_set_interface(struct unitytls_interface_struct* interface)
{
  unitytls = interface;
}

static bool unitytls_check_interface_available(struct Curl_easy* data)
{
  if(!unitytls) {
#if defined(UNITYTLS_LINKED)
    curl_unitytls_set_interface(unitytls_get_interface_struct());
#else
    if(data)
      failf(data, "UnityTls interface was not set. Call Curl_unitytls_set_interface first.");
    return false;
#endif
  }
  return true;
}

/*
* Implementation
*
*/

static char* load_file(const char* filepath, long* out_size)
{
  char* filecontent;
  FILE* file = fopen(filepath, "rb");

  if(!file)
    return NULL;

  fseek(file, 0, SEEK_END);
  *out_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  filecontent = malloc(*out_size);
  fread(filecontent, *out_size, 1, file);
  fclose(file);

  return filecontent;
}

static bool unitytls_append_pem_file(const char* filepath, unitytls_x509list* list, unitytls_errorstate* err)
{
  long fsize;
  char* filecontent = load_file(filepath, &fsize);
  if (!filecontent)
    return false;

  unitytls->unitytls_x509list_append_pem(list, filecontent, fsize, err);

  free(filecontent);
  return true;
}

static unitytls_key* unitytls_key_parse_pem_from_file(const char* filepath, const char* password, unitytls_errorstate* err)
{
  long fsize;
  unitytls_key* key = NULL;
  char* filecontent = load_file(filepath, &fsize);
  if (!filecontent)
    return NULL;

  unitytls->unitytls_key_parse_pem(filecontent, fsize, password, strlen(password), err);

  free(filecontent);
  return key;
}

static bool unitytls_parse_all_pem_in_dir(struct Curl_easy* data, const char* path, unitytls_x509list* list, unitytls_errorstate* err)
{
  bool success = false;
#if defined(WIN32)
  size_t len = strlen(path);
  WIN32_FIND_DATAW file_data;
  char pathFilename[MAX_PATH];
  HANDLE hFind;
  wchar_t* pathFilenameWChar = NULL;
  char* foundFilenameUTF8 = NULL;

  if(err->code != UNITYTLS_SUCCESS)
    return false;

  /* Path needs to end with '\*' */
  if(len + 2 >= MAX_PATH)
    return false;
  memset(pathFilename, 0, MAX_PATH);
  memcpy(pathFilename, path, len);
  pathFilename[len++] = '\\';
  pathFilename[len++] = '*';
  pathFilenameWChar = curlx_convert_UTF8_to_wchar(pathFilename);

  hFind = FindFirstFileExW(pathFilenameWChar, FindExInfoBasic, &file_data, FindExSearchNameMatch, NULL, 0);
  free(pathFilenameWChar);
  if(hFind == INVALID_HANDLE_VALUE)
    return CURLE_SSL_CACERT_BADFILE;

  do
  {
    if(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      continue;

    /* Try adding the file. Might or might not be a PEM file, so failure is not an error */
    foundFilenameUTF8 = curlx_convert_wchar_to_UTF8(file_data.cFileName);
    unitytls_append_pem_file(foundFilenameUTF8, list, err);
    free(foundFilenameUTF8);
    if(err->code != UNITYTLS_SUCCESS)
      *err = unitytls->unitytls_errorstate_create(); /* Need to reset to keep future falls to unitytls_append_pem_file working */
    else
      success = true;
  }
  while(FindNextFileW(hFind, &file_data) != 0);

  FindClose(hFind);
#else /* WIN32 */
  int snp_ret;
  struct dirent *entry;
  struct stat sb;
  char entry_name[512];
  DIR *dp;

  if(err->code != UNITYTLS_SUCCESS)
    return false;

  dp = opendir(path);
  if(dp == NULL)
    success = false;

  while((entry = readdir(dp)) != NULL) {
    snp_ret = snprintf(entry_name, sizeof(entry_name), "%s/%s", path, entry->d_name);
    if(snp_ret < 0 || (size_t)snp_ret >= sizeof(entry_name)) {
      break;
    }

    if(!S_ISREG(sb.st_mode))
      continue;

    /* Try adding the file. Might or might not be a PEM file, so failure is not an error */
    unitytls_append_pem_file(entry_name, list, err);
    if(err->code != UNITYTLS_SUCCESS)
      *err = unitytls->unitytls_errorstate_create(); /* Need to reset to keep future falls to unitytls_append_pem_file working */
    else
      success = true;
  }

  closedir(dp);
#endif /* WIN32 */

  return success;
}

static size_t unitytls_on_read(void* userData, UInt8* buf, size_t blen, unitytls_errorstate* errorState)
{
  struct Curl_cfilter *cf = (struct Curl_cfilter*)userData;
  struct Curl_easy *data = CF_DATA_CURRENT(cf);
  size_t nread;
  CURLcode result;

  /* OpenSSL catches this case, so should we. */
  if(!buf)
    return 0;

  DEBUGASSERT(data);

  result = Curl_conn_cf_recv(cf->next, data, (char *)buf, blen, &nread);
  if(CURLE_AGAIN == result) {
    unitytls->unitytls_errorstate_raise_error(errorState, UNITYTLS_USER_WOULD_BLOCK);
    return 0;
  }
  else if(result != CURLE_OK) {
    unitytls->unitytls_errorstate_raise_error(errorState, UNITYTLS_USER_READ_FAILED);
    return 0;
  }

  return (size_t)nread;
}

static size_t unitytls_on_write(void* userData, const UInt8* buf, size_t blen, unitytls_errorstate* errorState)
{
  struct Curl_cfilter *cf = (struct Curl_cfilter*)userData;
  struct Curl_easy *data = CF_DATA_CURRENT(cf);
  size_t nwritten;
  CURLcode result;

  DEBUGASSERT(data);
  result = Curl_conn_cf_send(cf->next, data, (char *)buf, blen, FALSE, &nwritten);
  if(CURLE_AGAIN == result) {
    unitytls->unitytls_errorstate_raise_error(errorState, UNITYTLS_USER_WOULD_BLOCK);
    return 0;
  }
  else if(result != CURLE_OK) {
    unitytls->unitytls_errorstate_raise_error(errorState, UNITYTLS_USER_WRITE_FAILED);
    return 0;
  }

  return (size_t)nwritten;
}

static void unitytls_on_cert_request(void* userData, unitytls_tlsctx* ctx,
                                   const char* cn, size_t cnLen,
                                   unitytls_x509name* caList, size_t caListLen,
                                   unitytls_x509list_ref* chain, unitytls_key_ref* key,
                                   unitytls_errorstate* errorState)
{
  struct ssl_backend_data* backend = ((struct ssl_connect_data*)userData)->backend;

  if(backend->clicert)
    *chain = unitytls->unitytls_x509list_get_ref(backend->clicert, errorState);
  if(backend->pk)
    *key = unitytls->unitytls_key_get_ref(backend->pk, errorState);
}

static unitytls_x509verify_result unitytls_on_verify(void* userData, unitytls_x509list_ref chain, unitytls_errorstate* errorState)
{
  struct Curl_cfilter *cf = (struct Curl_cfilter*)userData;
  struct ssl_connect_data *connssl = cf->ctx;
  struct ssl_backend_data* backend = connssl->backend;
  struct ssl_primary_config *conn_config = Curl_ssl_cf_get_primary_config(cf);
  const bool verifypeer = conn_config->verifypeer;
  const bool verifyhost = conn_config->verifyhost;
  const char* const hostname = connssl->peer.hostname;
  unitytls_x509verify_result verify_result = UNITYTLS_X509VERIFY_SUCCESS;

  /* According to documentation the options verifypeer and verifyhost are independent of each other! */
  /* UnityTls however, verifies both the certificate as well as the hostname in the same call. */
  if(verifypeer || verifyhost) {
    if(backend->cacert) {
      unitytls_x509list_ref trustCAref = unitytls->unitytls_x509list_get_ref(backend->cacert, errorState);
      verify_result = unitytls->unitytls_x509verify_explicit_ca(chain, trustCAref, hostname, strlen(hostname), NULL, NULL, errorState);
    }
    else {
      verify_result = unitytls->unitytls_x509verify_default_ca(chain, hostname, strlen(hostname), NULL, NULL, errorState);
    }

    /* Filter out special codes right away, so we can safely filter bitflags later on. */
    if(verify_result == UNITYTLS_X509VERIFY_NOT_DONE || verify_result == UNITYTLS_X509VERIFY_FATAL_ERROR)
      return verify_result;

    /* not interested in hostname verification */
    if(!verifyhost) {
      verify_result &= ~((unitytls_x509verify_result)UNITYTLS_X509VERIFY_FLAG_CN_MISMATCH);
    }
    /* only interested in hostname verification */
    else if(!verifypeer) {
      verify_result &=  ((unitytls_x509verify_result)UNITYTLS_X509VERIFY_FLAG_CN_MISMATCH);
    }
  }

  return verify_result;
}

static CURLcode unitytls_send(struct Curl_cfilter *cf, struct Curl_easy *data,
                             const void *mem, size_t len,
                             size_t *pnwritten)
{
  struct ssl_connect_data *connssl = cf->ctx;
  struct ssl_backend_data *backend = connssl->backend;
  CURLcode result = CURLE_OK;

  unitytls_errorstate err = unitytls->unitytls_errorstate_create();
  *pnwritten = unitytls->unitytls_tlsctx_write(backend->ctx, (const UInt8*)mem, len, &err);

  if(err.code != UNITYTLS_SUCCESS) {
    if(err.code == UNITYTLS_USER_WOULD_BLOCK)
      result = CURLE_AGAIN;
    else {
      result = CURLE_SEND_ERROR;
      failf(data, "Sending data failed with unitytls error code %i", err.code);
    }
    return result;
  }

  return result;
}

static CURLcode unitytls_recv(struct Curl_cfilter *cf, struct Curl_easy *data,
                             char *buf, size_t buffersize,
                             size_t *pnread)
{
  struct ssl_connect_data *connssl = cf->ctx;
  struct ssl_backend_data *backend = connssl->backend;
  CURLcode result = CURLE_OK;

  unitytls_errorstate err = unitytls->unitytls_errorstate_create();

  *pnread = unitytls->unitytls_tlsctx_read(backend->ctx, (UInt8*)buf, buffersize, &err);

  // Curl expects us to ignore gracefully closed connections on read.
  if(err.code == UNITYTLS_STREAM_CLOSED)
    return 0;

  if(err.code != UNITYTLS_SUCCESS) {
    if(err.code == UNITYTLS_USER_WOULD_BLOCK)
      result = CURLE_AGAIN;
    else {
      result = CURLE_RECV_ERROR;
      failf(data, "Receiving data failed with unitytls error code %i", err.code);
    }
    return result;
  }

  return result;
}

typedef struct {
    FILE *file;
    int in_client_random : 1;
    int in_master_secret : 1;
    int hexdump_lines_to_process;
} keylog_debug_ctx;
static FILE *debug_log_file = NULL;

static void unitytls_keylog_debug_callback (void* userData, unitytls_tlsctx* ctx, int level, char const* fileName, size_t lineNumber, const char* str)
{
    keylog_debug_ctx *c = (keylog_debug_ctx *) userData;

    if (strstr(str, "dumping 'client hello, random bytes' (32 bytes)")) {
        c->in_client_random = 1;
        c->hexdump_lines_to_process = 2;
        fputs("CLIENT_RANDOM ", c->file);
        return;
    } else if (strstr(str, "dumping 'master secret' (48 bytes)")) {
        c->in_master_secret = 1;
        c->hexdump_lines_to_process = 3;
        fputc(' ', c->file);
        return;
    } else if ((!c->in_client_random && !c->in_master_secret) ||
               c->hexdump_lines_to_process == 0) {
        return;
    }

    /* Parse "0000:  64 df 18 71 ca 4a 4b e4 63 87 2a ef 5f 29 ca ff  ..." */
    str = strstr(str, ":  ");
    if (!str || strlen(str) < 3 + 3*16) {
        goto reset;         /* not the expected hex buffer */
    }
    str += 3;               /* skip over ":  " */

    /* Process sequences of "hh " */
    for (int i = 0; i < 3 * 16; i += 3) {
        char c1 = str[i], c2 = str[i + 1], c3 = str[i + 2];
        if ((('0' <= c1 && c1 <= '9') || ('a' <= c1 && c1 <= 'f')) &&
            (('0' <= c2 && c2 <= '9') || ('a' <= c2 && c2 <= 'f')) &&
            c3 == ' ') {
            fputc(c1, c->file);
            fputc(c2, c->file);
        } else {
            goto reset;     /* unexpected non-hex char */
        }
    }

    if (--c->hexdump_lines_to_process != 0 || !c->in_master_secret) {
        return;             /* line is not yet finished. */
    }

reset:
    c->hexdump_lines_to_process = 0;
    c->in_client_random = c->in_master_secret = 0;
    fputc('\n', c->file);   /* finish key log line */
    fflush(c->file);
}

static CURLcode unitytls_connect_step1(struct Curl_cfilter *cf, struct Curl_easy *data)
{
  struct ssl_connect_data *connssl = cf->ctx;
  struct ssl_backend_data *backend = connssl->backend;
  struct ssl_primary_config *conn_config = Curl_ssl_cf_get_primary_config(cf);
  const struct curl_blob *ca_info_blob = conn_config->ca_info_blob;
  struct ssl_config_data *ssl_config = Curl_ssl_cf_get_config(cf, data);
  const char* const ssl_cafile = (ca_info_blob ? NULL : conn_config->CAfile);
  const bool verifypeer = conn_config->verifypeer;
  const char* const ssl_capath = conn_config->CApath;
  char* const ssl_cert = ssl_config->primary.clientcert;
  const char* const hostname = connssl->peer.hostname;

  unitytls_errorstate err = unitytls->unitytls_errorstate_create();

  unitytls_tlsctx_callbacks callbacks = { unitytls_on_read, unitytls_on_write, cf };

  /* unitytls only supports TLS 1.0-1.2 */
  switch (conn_config->version)
  {
    case CURL_SSLVERSION_DEFAULT:
    case CURL_SSLVERSION_TLSv1_0:
    case CURL_SSLVERSION_TLSv1_1:
    case CURL_SSLVERSION_TLSv1_2:
      break;
    default:
      failf(data, "unitytls only supports TLS 1.0-1.2");
      return CURLE_SSL_CONNECT_ERROR;
  }

  /* Load the trusted CA */
  if (ssl_cafile || ssl_capath)
    backend->cacert = unitytls->unitytls_x509list_create(&err);

  if(ssl_cafile) {
    if(!unitytls_append_pem_file(ssl_cafile, backend->cacert, &err) || err.code != UNITYTLS_SUCCESS) {
      failf(data, "Error reading ca cert file from %s", ssl_cafile);
      if(verifypeer)
        return CURLE_SSL_CACERT_BADFILE;
      err = unitytls->unitytls_errorstate_create(); /* ignore any errors that came up */
    }
  }

  if(ssl_capath) {
    if(!unitytls_parse_all_pem_in_dir(data, ssl_capath, backend->cacert, &err) || err.code != UNITYTLS_SUCCESS) {
      failf(data, "Error reading ca cert path from %s", ssl_cafile);
      if(verifypeer)
        return CURLE_SSL_CACERT_BADFILE;
      err = unitytls->unitytls_errorstate_create(); /* ignore any errors that came up */
    }
  }

  /* Load the client certificate */
  if(ssl_cert) {
    backend->clicert = unitytls->unitytls_x509list_create(&err);
    if(unitytls_append_pem_file(ssl_cert, backend->clicert, &err) != CURLE_OK || err.code != UNITYTLS_SUCCESS) {
      failf(data, "Error reading client cert file %s", ssl_cafile);
      return CURLE_SSL_CERTPROBLEM;
    }
  }

  /* Load the client private key */
  if(ssl_config->key) {
    backend->pk = unitytls_key_parse_pem_from_file(ssl_config->key, ssl_config->key_passwd, &err);
    if(!backend->pk || err.code != UNITYTLS_SUCCESS) {
      failf(data, "Error reading private key %s", ssl_config->key);
      return CURLE_SSL_CERTPROBLEM;
    }
  }
  else {
    backend->pk = NULL;
  }

  unitytls_tlsctx_protocolrange protocol_range;

  /* Create and configure context */
  switch(conn_config->version) {
    case CURL_SSLVERSION_DEFAULT:
      protocol_range = unitytls->UNITYTLS_TLSCTX_PROTOCOLRANGE_DEFAULT;
      break;
    case CURL_SSLVERSION_TLSv1_0:
      protocol_range.max = protocol_range.min = UNITYTLS_PROTOCOL_TLS_1_0;
      break;
    case CURL_SSLVERSION_TLSv1_1:
      protocol_range.max = protocol_range.min = UNITYTLS_PROTOCOL_TLS_1_1;
      break;
    case CURL_SSLVERSION_TLSv1_2:
      protocol_range.max = protocol_range.min = UNITYTLS_PROTOCOL_TLS_1_2;
      break;
    default:
      failf(data, "Unrecognized/unsupported parameter passed via CURLOPT_SSLVERSION");
      return CURLE_SSL_CONNECT_ERROR;
  }

  keylog_debug_ctx *debug_ctx = NULL;
  char* ssl_keylog_filename = curl_getenv("UNITY_CURL_SSL_KEYLOG");
  if (ssl_keylog_filename != NULL) {
    debug_ctx = malloc(sizeof(keylog_debug_ctx));
    memset(debug_ctx, 0, sizeof(keylog_debug_ctx));
    if (debug_log_file == NULL) {
      debug_log_file = fopen(ssl_keylog_filename, "a");
    }
    debug_ctx->file = debug_log_file;
    curl_free(ssl_keylog_filename);
  }

  backend->ctx = unitytls->unitytls_tlsctx_create_client(protocol_range, callbacks, hostname, strlen(hostname), &err);
  if (debug_ctx != NULL) {
    unitytls->unitytls_tlsctx_set_trace_level(backend->ctx, UNITYTLS_LOGLEVEL_TRACE);
    unitytls->unitytls_tlsctx_set_trace_callback(backend->ctx, unitytls_keylog_debug_callback, debug_ctx, &err);
  }
  unitytls->unitytls_tlsctx_set_certificate_callback(backend->ctx, unitytls_on_cert_request, connssl, &err);
  unitytls->unitytls_tlsctx_set_x509verify_callback(backend->ctx, unitytls_on_verify, cf, &err);
  if(err.code != UNITYTLS_SUCCESS) {
    failf(data, "Error creating and configuring untiytls context: %i", err.code);
    return CURLE_SSL_CONNECT_ERROR;
  }

#ifdef HAS_ALPN
  if (connssl->alpn) {
    struct alpn_proto_buf proto;
    // mbedtls_ssl_conf_alpn_protocols does not clone the protocols array, which is why we need to keep it inside backend struct
    size_t i;
    DEBUGASSERT(connssl->alpn->count <= ALPN_ENTRIES_MAX);
    for (i = 0; i < connssl->alpn->count; ++i) {
      backend->protocols[i] = connssl->alpn->entries[i];
    }
    backend->protocols[connssl->alpn->count] = NULL; // the protocols array must be null-terminated
    unitytls->unitytls_tlsctx_set_alpn_protocols(backend->ctx, &backend->protocols[0], &err);
    if(err.code != UNITYTLS_SUCCESS) {
      failf(data, "Failed setting APLN protocols: %i", err.code);
      return CURLE_SSL_CONNECT_ERROR;
    }
    Curl_alpn_to_proto_str(&proto, connssl->alpn);
    infof(data, VTLS_INFOF_ALPN_OFFER_1STR, proto.data);
  }
#endif

  /* give application a chance to interfere with SSL set up. */
  if(data->set.ssl.fsslctx) {
    CURLcode result = (*data->set.ssl.fsslctx)(data, backend->ctx, data->set.ssl.fsslctxp);
    if(result != CURLE_OK) {
      failf(data, "error signaled by ssl ctx callback");
      return result;
    }
  }

  connssl->connecting_state = ssl_connect_2;

  return CURLE_OK;
}

static CURLcode unitytls_connect_step2(struct Curl_cfilter* cf, struct Curl_easy* data, struct ssl_connect_data* connssl)
{
  struct ssl_backend_data* backend = connssl->backend;

  unitytls_errorstate err = unitytls->unitytls_errorstate_create();
  unitytls_x509verify_result verifyresult = unitytls->unitytls_tlsctx_process_handshake(backend->ctx, &err);
  CURLcode result = CURLE_OK;

  if (err.code == UNITYTLS_USER_WOULD_BLOCK) {
    return CURLE_OK;  /* all fine but no state change yet */
  }

  if(verifyresult != UNITYTLS_X509VERIFY_SUCCESS) {
    if(verifyresult == UNITYTLS_X509VERIFY_FATAL_ERROR) {
      failf(data, "Cert handshake failed. %s. UnityTls error code: %i", 
        unitytls->unitytls_x509verify_result_to_string(verifyresult), err.code);
      return CURLE_SSL_CONNECT_ERROR;
    }
    else {
      for (int i = 0; i < 32; ++i) {
        const uint32_t mask = 1 << i;
        if(verifyresult & mask)
          failf(data, "Cert verify failed. %s. UnityTls error code: %i",
                unitytls->unitytls_x509verify_result_to_string(mask), err.code);
      }
      if(verifyresult & UNITYTLS_X509VERIFY_FLAG_REVOKED) {
        return CURLE_SSL_CACERT_BADFILE;
      }
      /* Note that UNITYTLS_X509VERIFY_NOT_DONE is always always an error as well since we are never running in server mode (unitytls_tlsctx_create_server)
       * which means that authentification method should always be called. 
       * However, this usually has a different reason so it is not CURLE_PEER_FAILED_VERIFICATION */
      if (verifyresult == UNITYTLS_X509VERIFY_NOT_DONE) {
        failf(data, "Cert handshake failed. %s. UnityTls error code: %i", 
            unitytls->unitytls_x509verify_result_to_string(verifyresult), err.code);
        return CURLE_SSL_CONNECT_ERROR;
      }
      else
        return CURLE_PEER_FAILED_VERIFICATION;
    }
  }

#ifdef HAS_ALPN
  if (connssl->alpn) {
    const char *proto = unitytls->unitytls_tlsctx_get_alpn_protocol(backend->ctx);
    CURLcode result = Curl_alpn_set_negotiated(cf, data, connssl,
                                              (const unsigned char *)proto,
                                              proto ? strlen(proto) : 0);
    if(result)
      return result;
  }
#endif

  /* We almost certainly have a verifyresult!=UNITYTLS_X509VERIFY_SUCCESS as well, but in theory it is still possible to hit this code. */
  if (err.code == UNITYTLS_SUCCESS) {
    connssl->connecting_state = ssl_connect_3;
    return CURLE_OK;
  }
  else {
    failf(data, "Handshake failed. UnityTls error code: %i", err.code);
    return CURLE_SSL_CONNECT_ERROR;
  }
}

static CURLcode unitytls_connect_step3(struct ssl_connect_data* connssl)
{
  /* TODO: Session suppport. */
  connssl->connecting_state = ssl_connect_done;
  return CURLE_OK;
}

static CURLcode unitytls_connect_common(struct Curl_cfilter *cf, struct Curl_easy *data,
                                        bool nonblocking,
                                        bool *done)
{
  CURLcode retcode;
  struct ssl_connect_data* connssl = cf->ctx;
  curl_socket_t sockfd = Curl_conn_cf_get_socket(cf, data);

  if(!unitytls_check_interface_available(data))
    return CURLE_USE_SSL_FAILED;

  /* check if the connection has already been established */
  if(ssl_connection_complete == connssl->state) {
    *done = true;
    return CURLE_OK;
  }

  if(ssl_connect_1 == connssl->connecting_state) {
    /* Find out how much more time we're allowed */
    if(Curl_timeleft(data, NULL, true) < 0) {
      /* no need to continue if time already is up */
      failf(data, "SSL connection timeout");
      return CURLE_OPERATION_TIMEDOUT;
    }
    retcode = unitytls_connect_step1(cf, data);
    if(retcode)
      return retcode;
  }


  while(ssl_connect_2 == connssl->connecting_state) {

    /* check allowed time left */
    if(Curl_timeleft(data, NULL, TRUE) < 0) {
      /* no need to continue if time already is up */
      failf(data, "SSL connection timeout");
      return CURLE_OPERATION_TIMEDOUT;
    }

    retcode = unitytls_connect_step2(cf, data, connssl);
    if(retcode != CURLE_OK || (nonblocking && ssl_connect_2 == connssl->connecting_state))
      return retcode;
  } /* repeat step2 until all transactions are done. */

  if(ssl_connect_3 == connssl->connecting_state) {
    retcode = unitytls_connect_step3(connssl);
    if(retcode)
      return retcode;
  }

  if(ssl_connect_done==connssl->connecting_state) {
    connssl->state = ssl_connection_complete;
    *done = TRUE;
  }
  else
    *done = FALSE;

  /* Reset our connect state machine */
  connssl->connecting_state = ssl_connect_1;

  return CURLE_OK;
}

static CURLcode Curl_unitytls_connect_nonblocking(struct Curl_cfilter *cf, struct Curl_easy *data, bool *done)
{
  return unitytls_connect_common(cf, data, true, done);
}

static void Curl_unitytls_close(struct Curl_cfilter *cf, struct Curl_easy *data)
{
  struct ssl_connect_data* connssl = cf->ctx;
  struct ssl_backend_data* backend = connssl->backend;

  unitytls_errorstate err;
  if(!unitytls_check_interface_available(NULL))
    return;

  if (backend->ctx) {
    err = unitytls->unitytls_errorstate_create();

    /* Read any received close notify to avoid RST on TCP connection. */
    UInt8 buf[32];
    (void)unitytls->unitytls_tlsctx_read(backend->ctx, buf, sizeof(buf), &err);

    unitytls->unitytls_tlsctx_free(backend->ctx);
    backend->ctx = NULL;
  }

  unitytls->unitytls_x509list_free(backend->cacert);
  backend->cacert = NULL;
  unitytls->unitytls_x509list_free(backend->clicert);
  backend->clicert = NULL;
  unitytls->unitytls_key_free(backend->pk);
  backend->pk = NULL;
}

static size_t Curl_unitytls_version(char *buffer, size_t size)
{
  return snprintf(buffer, size, "UnityTls");
}

static CURLcode Curl_unitytls_random(struct Curl_easy *data, unsigned char *entropy, size_t length)
{
  unitytls_errorstate err;

  if(!unitytls_check_interface_available(data))
    return CURLE_FAILED_INIT;

  err = unitytls->unitytls_errorstate_create();
  unitytls->unitytls_random_generate_bytes(entropy, length, &err);

  if(err.code != UNITYTLS_SUCCESS)
    return CURLE_FAILED_INIT;

  return CURLE_OK;
}

static void *Curl_unitytls_get_internals(struct ssl_connect_data *connssl,
                                        CURLINFO info)
{
  struct ssl_backend_data *backend = connssl->backend;
  (void)info;
  return backend->ctx;
}

const struct Curl_ssl Curl_ssl_unitytls = {
  { CURLSSLBACKEND_UNITYTLS, "unitytls" }, /* info */

  SSLSUPP_CA_PATH |
  SSLSUPP_SSL_CTX,

  sizeof(struct ssl_backend_data),

  NULL,                             /* init */
  NULL,                             /* cleanup */
  Curl_unitytls_version,            /* version */
  NULL,                             /* shutdown */
  NULL,                             /* data_pending */
  Curl_unitytls_random,             /* random */
  NULL,                             /* cert_status_request */
  Curl_unitytls_connect_nonblocking,/* connect */
  Curl_ssl_adjust_pollset,          /* adjust_pollset */
  Curl_unitytls_get_internals,      /* get_internals */
  Curl_unitytls_close,              /* close_one */
  NULL,                             /* close_all */
  NULL,                             /* set_engine */
  NULL,                             /* set_engine_default */
  NULL,                             /* engines_list */
  NULL,                             /* sha256sum */
  unitytls_recv,                    /* recv_plain */
  unitytls_send,                    /* send_plain */
  NULL,                             /* get_channel_binding */
};

#endif /* USE_UNITYTLS */