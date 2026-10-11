#ifndef HEADER_CURL_CONFIG_WIN32_H
#define HEADER_CURL_CONFIG_WIN32_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/

/* Handcrafted config file for building via Visual Studio IDE Project Files */

#if !defined(_MSC_VER) || _MSC_VER > 1800
#error This manual configuration requires MSVC 2010-2013 (IDE Project builds)
#endif

/* Define if you have the <sys/param.h> header file. */
#ifdef __MINGW32__
#define HAVE_SYS_PARAM_H 1
#endif

#define HAVE_ZLIB_H 1
#define HAVE_LIBZ 1

/* Define if you have the <unistd.h> header file. */
#ifdef __MINGW32__
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the <libgen.h> header file. */
#ifdef __MINGW32__
#define HAVE_LIBGEN_H 1
#endif

/* ---------------------------------------------------------------- */
/*                        OTHER HEADER INFO                         */
/* ---------------------------------------------------------------- */

/* Define if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* ---------------------------------------------------------------- */
/*                             FUNCTIONS                            */
/* ---------------------------------------------------------------- */

/* Define to 1 if you have the getsockname function. */
#define HAVE_GETSOCKNAME 1

/* Define if you have the gettimeofday function. */
#ifdef __MINGW32__
#define HAVE_GETTIMEOFDAY 1
#endif

/* Define if you have the ioctlsocket function. */
#define HAVE_IOCTLSOCKET 1

/* Define if you have a working ioctlsocket FIONBIO function. */
#define HAVE_IOCTLSOCKET_FIONBIO 1

/* Define if you have the select function. */
#define HAVE_SELECT 1

/* Define if you have the setlocale function. */
#define HAVE_SETLOCALE 1

/* Define if you have the socket function. */
#define HAVE_SOCKET 1

/* Define if you have the utime function. */
#define HAVE_UTIME 1

/* Define if you have the recv function. */
#define HAVE_RECV 1

/* Define to the type of arg 1 for recv. */
#define RECV_TYPE_ARG1 SOCKET

/* Define to the type of arg 2 for recv. */
#define RECV_TYPE_ARG2 char *

/* Define to the type of arg 3 for recv. */
#define RECV_TYPE_ARG3 int

/* Define to the type of arg 4 for recv. */
#define RECV_TYPE_ARG4 int

/* Define to the function return type for recv. */
#define RECV_TYPE_RETV int

/* Define if you have the send function. */
#define HAVE_SEND 1

/* Define to the type of arg 1 for send. */
#define SEND_TYPE_ARG1 SOCKET

/* Define to the type of arg 2 for send. */
#define SEND_TYPE_ARG2 char *

/* Define to the type of arg 3 for send. */
#define SEND_TYPE_ARG3 int

/* Define to the type of arg 4 for send. */
#define SEND_TYPE_ARG4 int

/* Define to the function return type for send. */
#define SEND_TYPE_RETV int

/* Must always use local implementations on Windows. */
/* Define to 1 if you have an IPv6 capable working inet_ntop function. */
/* #undef HAVE_INET_NTOP */
/* Define to 1 if you have an IPv6 capable working inet_pton function. */
/* #undef HAVE_INET_PTON */

/* Define to 1 if you have the `basename' function. */
#ifdef __MINGW32__
#define HAVE_BASENAME 1
#endif

/* Define to 1 if you have the signal function. */
#define HAVE_SIGNAL 1

/* ---------------------------------------------------------------- */
/*                       TYPEDEF REPLACEMENTS                       */
/* ---------------------------------------------------------------- */

/* Define if ssize_t is not an available 'typedefed' type. */
#ifndef _SSIZE_T_DEFINED
#  ifdef __MINGW32__
#  elif defined(_WIN64)
#    define _SSIZE_T_DEFINED
#    define ssize_t __int64
#  else
#    define _SSIZE_T_DEFINED
#    define ssize_t int
#  endif
#endif

/* ---------------------------------------------------------------- */
/*                            TYPE SIZES                            */
/* ---------------------------------------------------------------- */

/* Define to the size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* Define to the size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* Define to the size of `size_t', as computed by sizeof. */
#ifdef _WIN64
#  define SIZEOF_SIZE_T 8
#else
#  define SIZEOF_SIZE_T 4
#endif

/* Define to the size of `curl_off_t', as computed by sizeof. */
#define SIZEOF_CURL_OFF_T 8

/* ---------------------------------------------------------------- */
/*                        COMPILER SPECIFIC                         */
/* ---------------------------------------------------------------- */

/* Default to 64-bit time_t unless _USE_32BIT_TIME_T is defined */
#if defined(_MSC_VER) || defined(__MINGW32__)
#  ifndef _USE_32BIT_TIME_T
#    define SIZEOF_TIME_T 8
#  else
#    define SIZEOF_TIME_T 4
#  endif
#endif

/* ---------------------------------------------------------------- */
/*                          STRUCT RELATED                          */
/* ---------------------------------------------------------------- */

/* Define if you have struct sockaddr_storage. */
#define HAVE_STRUCT_SOCKADDR_STORAGE 1

/* Define if you have struct timeval. */
#define HAVE_STRUCT_TIMEVAL 1

/* Define if struct sockaddr_in6 has the sin6_scope_id member. */
#define HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID 1

/* ---------------------------------------------------------------- */
/*                        LARGE FILE SUPPORT                        */
/* ---------------------------------------------------------------- */

/* Number of bits in a file offset, on hosts where this is settable. */
#ifdef __MINGW32__
#  undef _FILE_OFFSET_BITS
#  define _FILE_OFFSET_BITS 64
#endif

/* Define to the size of `off_t', as computed by sizeof. */
#ifdef __MINGW32__
#  define SIZEOF_OFF_T 8
#else
#  define SIZEOF_OFF_T 4
#endif

/*
 * Headers and functions
 */

#define HAVE_FCNTL_H 1
#define HAVE_IO_H 1
#define HAVE_LOCALE_H 1
#if _MSC_VER >= 1800
#define HAVE_STDBOOL_H 1
#define HAVE_BOOL_T 1
#endif
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_UTIME_H 1

#define HAVE_CLOSESOCKET 1
#define HAVE_FREEADDRINFO 1
#define HAVE_GETADDRINFO 1
#define HAVE_GETADDRINFO_THREADSAFE 1
#define HAVE_GETHOSTNAME 1
#define HAVE_GETPEERNAME 1
#define HAVE_GETSOCKNAME 1
#define HAVE_IOCTLSOCKET 1
#define HAVE_IOCTLSOCKET_FIONBIO 1
#define HAVE_SETLOCALE 1
#define HAVE_SOCKET 1
#define HAVE_UTIME 1
#define HAVE_RECV 1
#define RECV_TYPE_ARG1 SOCKET
#define RECV_TYPE_ARG2 char *
#define RECV_TYPE_ARG3 int
#define RECV_TYPE_ARG4 int
#define RECV_TYPE_RETV int
#define HAVE_SEND 1
#define SEND_TYPE_ARG1 SOCKET
#define SEND_TYPE_ARG2 char *
#define SEND_TYPE_ARG3 int
#define SEND_TYPE_ARG4 int
#define SEND_TYPE_RETV int
#define HAVE_SIGNAL 1

/*
 * Types and sizes
 */

#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#ifdef _WIN64
#  define SIZEOF_SIZE_T 8
#  define ssize_t __int64
#else
#  define SIZEOF_SIZE_T 4
#  define ssize_t int
#endif
#define SIZEOF_CURL_OFF_T 8
/* Default to 64-bit time_t unless _USE_32BIT_TIME_T is defined */
#ifndef _USE_32BIT_TIME_T
#  define SIZEOF_TIME_T 8
#else
#  define SIZEOF_TIME_T 4
#endif
#define SIZEOF_OFF_T 4

#define HAVE_STRUCT_SOCKADDR_STORAGE 1
#define HAVE_STRUCT_TIMEVAL 1
#define HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID 1

/*
 * Additional definitions
 */

/* Default define to enable threaded asynchronous DNS lookups. */
#if !defined(USE_RESOLV_THREADED) && !defined(USE_SYNC_DNS)
#  define USE_RESOLV_THREADED 1
#endif

#define HAVE_LDAP_SSL 1
#define USE_WIN32_LDAP 1
#define USE_WIN32_CRYPTO 1
#define USE_UNIX_SOCKETS 1

#ifndef CURL_OS
#  ifdef _M_IX86
#  define CURL_OS "i386-pc-win32"
#  elif defined(_M_X64)
#  define CURL_OS "x86_64-pc-win32"
#  else
#  define CURL_OS "unknown-pc-win32"
#  endif
#endif

#endif /* HEADER_CURL_CONFIG_WIN32_H */
