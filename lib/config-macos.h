// Originally created by:
// ./configure --disable-sftp --disable-scp --disable-ldap --disable-ldaps --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp --disable-gopher --disable-manual --disable-verbose --disable-sspi --disable-unix-sockets --disable-versioned-symbols --without-ssl --with-unitytls --without-libidn --without-libpsl --without-libssh2 --enable-threaded-resolver --enable-static --disable-shared
// Some manual changes on top of this (after empty line at the end).

#define CURL_DISABLE_DICT 1
#define CURL_DISABLE_GOPHER 1
#define CURL_DISABLE_HSTS 1
#define CURL_DISABLE_IMAP 1
#define CURL_DISABLE_LDAP 1
#define CURL_DISABLE_LDAPS 1
#define CURL_DISABLE_POP3 1
#define CURL_DISABLE_SMTP 1
#define CURL_DISABLE_TELNET 1
#define CURL_DISABLE_TFTP 1
#define CURL_DISABLE_VERBOSE_STRINGS 1
#define CURL_EXTERN_SYMBOL __attribute__ ((__visibility__ ("default")))
#define CURL_SA_FAMILY_T sa_family_t
#define ENABLE_IPV6 1
#define GETHOSTNAME_TYPE_ARG2 size_t
#define HAVE_ALARM 1
#define HAVE_ALLOCA_H 1
#define HAVE_ARPA_INET_H 1
#define HAVE_ARPA_TFTP_H 1
#define HAVE_ASSERT_H 1
#define HAVE_ATOMIC 1
#define HAVE_BASENAME 1
#define HAVE_BOOL_T 1
#define HAVE_BUILTIN_AVAILABLE 1
#define HAVE_CLOCK_GETTIME_MONOTONIC 1
#define HAVE_CONNECT 1
#define HAVE_DECL_GETPWUID_R 1
#define HAVE_DLFCN_H 1
#define HAVE_ERRNO_H 1
#define HAVE_FCHMOD 1
#define HAVE_FCNTL 1
#define HAVE_FCNTL_H 1
#define HAVE_FCNTL_O_NONBLOCK 1
#define HAVE_FNMATCH 1
#define HAVE_FREEADDRINFO 1
#define HAVE_FSETXATTR 1
#define HAVE_FSETXATTR_6 1
#define HAVE_FTRUNCATE 1
#define HAVE_GETADDRINFO 1
#define HAVE_GETADDRINFO_THREADSAFE 1
#define HAVE_GETEUID 1
#define HAVE_GETHOSTBYNAME 1
#define HAVE_GETHOSTNAME 1
#define HAVE_GETIFADDRS 1
#define HAVE_GETPEERNAME 1
#define HAVE_GETPPID 1
#define HAVE_GETPWUID 1
#define HAVE_GETPWUID_R 1
#define HAVE_GETRLIMIT 1
#define HAVE_GETSOCKNAME 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_GMTIME_R 1
#define HAVE_IFADDRS_H 1
#define HAVE_IF_NAMETOINDEX 1
#define HAVE_INET_NTOP 1
#define HAVE_INET_PTON 1
#define HAVE_INTTYPES_H 1
#define HAVE_IOCTL 1
#define HAVE_IOCTL_FIONBIO 1
#define HAVE_IOCTL_SIOCGIFADDR 1
#define HAVE_LIBGEN_H 1
#define HAVE_LIBZ 1
#define HAVE_LL 1
#define HAVE_LOCALE_H 1
#define HAVE_LOCALTIME_R 1
#define HAVE_LONGLONG 1
#define HAVE_MACH_ABSOLUTE_TIME 1
#define HAVE_MSG_NOSIGNAL 1
#define HAVE_NETDB_H 1
#define HAVE_NETINET_IN_H 1
#define HAVE_NETINET_TCP_H 1
#define HAVE_NETINET_UDP_H 1
#define HAVE_NET_IF_H 1
#define HAVE_PIPE 1
#define HAVE_POLL_H 1
#define HAVE_POSIX_STRERROR_R 1
#define HAVE_PTHREAD_H 1
#define HAVE_PWD_H 1
#define HAVE_RECV 1
#define HAVE_SCHED_YIELD 1
#define HAVE_SELECT 1
#define HAVE_SEND 1
#define HAVE_SENDMSG 1
#define HAVE_SETJMP_H 1
#define HAVE_SETLOCALE 1
#define HAVE_SETMODE 1
#define HAVE_SETRLIMIT 1
#define HAVE_SETSOCKOPT 1
#define HAVE_SIGACTION 1
#define HAVE_SIGINTERRUPT 1
#define HAVE_SIGNAL 1
#define HAVE_SIGNAL_H 1
#define HAVE_SIGSETJMP 1
#define HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID 1
#define HAVE_SOCKET 1
#define HAVE_SOCKETPAIR 1
#define HAVE_STDBOOL_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRCASECMP 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR_R 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_STRSTR 1
#define HAVE_STRTOK_R 1
#define HAVE_STRTOLL 1
#define HAVE_STRUCT_SOCKADDR_STORAGE 1
#define HAVE_STRUCT_TIMEVAL 1
#define HAVE_SUSECONDS_T 1
#define HAVE_SYS_FILIO_H 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_POLL_H 1
#define HAVE_SYS_RESOURCE_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_SOCKIO_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_UIO_H 1
#define HAVE_SYS_UN_H 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_SYS_XATTR_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_UNISTD_H 1
#define HAVE_USLEEP 1
#define HAVE_UTIME 1
#define HAVE_UTIMES 1
#define HAVE_UTIME_H 1
#define HAVE_VARIADIC_MACROS_C99 1
#define HAVE_VARIADIC_MACROS_GCC 1
#define HAVE_WRITABLE_ARGV 1
#define HAVE_WRITEV 1
#define HAVE_ZLIB_H 1
#define LT_OBJDIR ".libs/"
#define OS "x86_64-apple-darwin21.6.0"
#define PACKAGE "curl"
#define PACKAGE_BUGREPORT "a suitable curl mailing list: https://curl.se/mail/"
#define PACKAGE_NAME "curl"
#define PACKAGE_STRING "curl -"
#define PACKAGE_TARNAME "curl"
#define PACKAGE_URL ""
#define PACKAGE_VERSION "-"
#define RECV_TYPE_ARG1 int
#define RECV_TYPE_ARG2 void *
#define RECV_TYPE_ARG3 size_t
#define RECV_TYPE_ARG4 int
#define RECV_TYPE_RETV ssize_t
#define SELECT_QUAL_ARG5 
#define SELECT_TYPE_ARG1 int
#define SELECT_TYPE_ARG234 fd_set *
#define SELECT_TYPE_ARG5 struct timeval *
#define SELECT_TYPE_RETV int
#define SEND_QUAL_ARG2 const
#define SEND_TYPE_ARG1 int
#define SEND_TYPE_ARG2 void *
#define SEND_TYPE_ARG3 size_t
#define SEND_TYPE_ARG4 int
#define SEND_TYPE_RETV ssize_t
#define SIZEOF_CURL_OFF_T 8
#define SIZEOF_INT 4
#define SIZEOF_LONG 8
#define SIZEOF_OFF_T 8
#define SIZEOF_SHORT 2
#define SIZEOF_SIZE_T 8
#define SIZEOF_TIME_T 8
#define STDC_HEADERS 1
#define STRERROR_R_TYPE_ARG3 size_t
#define USE_THREADS_POSIX 1
#define VERSION "-"
#ifndef _ALL_SOURCE
#endif
#ifndef __cplusplus
#endif

#define USE_UNITYTLS 1
// our curl version disabled poll on macOS due to bugs in macOS 10.12
// Unity has no need for supporting macOS 10.12
// and, we do need to support contexts with >1k open files
// curl also enables HAVE_POLL on macOS in upstream
// see https://github.com/curl/curl/issues/1057 for historical context 
// nb: newer curl releases *do* use poll on macOS, so we can just regenerate once we sync
#define HAVE_POLL 1
#define HAVE_POLL_FINE 1
