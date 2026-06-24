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
#include "tool_setup.h"

#include "tool_util.h"

#ifdef _WIN32

struct timeval tvrealnow(void)
{
  /* UNIX EPOCH (1970-01-01) in FILETIME (1601-01-01) as 64-bit value */
  static const uint64_t EPOCH = UINT64_C(116444736000000000);
  SYSTEMTIME systime;
  FILETIME ftime; /* 100ns since 1601-01-01, as double 32-bit value */
  uint64_t time; /* 100ns since 1601-01-01, as 64-bit value */
  struct timeval now;
#ifndef CURL_WINDOWS_APP
  if(tool_isVistaOrGreater) { /* QPC timer might have issues pre-Vista */
#endif
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    now.tv_sec = (long)(count.QuadPart / tool_freq.QuadPart);
    now.tv_usec = (long)((count.QuadPart % tool_freq.QuadPart) * 1000000 /
                         tool_freq.QuadPart);
#ifndef CURL_WINDOWS_APP
  }
  else {
    /* Disable /analyze warning that GetTickCount64 is preferred  */
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:28159)
#endif
    DWORD milliseconds = GetTickCount();
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    now.tv_sec = (long)(milliseconds / 1000);
    now.tv_usec = (long)((milliseconds % 1000) * 1000);
  }
#endif
  return now;
}

#else

struct timeval tvrealnow(void)
{
  struct timeval now;
#ifdef HAVE_GETTIMEOFDAY
  (void)gettimeofday(&now, NULL);
#else
  now.tv_sec = time(NULL);
  now.tv_usec = 0;
#endif
  return now;
}

#endif

/* Case insensitive compare. Accept NULL pointers. */
int struplocompare(const char *p1, const char *p2)
{
  if(!p1)
    return p2 ? -1 : 0;
  if(!p2)
    return 1;
  return CURL_STRICMP(p1, p2);
}

/* Indirect version to use as qsort callback. */
int struplocompare4sort(const void *p1, const void *p2)
{
  return struplocompare(*(char * const *)p1, *(char * const *)p2);
}

#if defined(_WIN32) && !defined(__MINGW32__)
/*
 * Truncate a file handle at a 64-bit position 'where'.
 */
int toolx_ftruncate_win32(int fd, curl_off_t where)
{
  intptr_t handle = _get_osfhandle(fd);

  if(curl_lseek(fd, where, SEEK_SET) == LSEEK_ERROR)
    return -1;

  if(!SetEndOfFile((HANDLE)handle))
    return -1;

  return 0;
}
#elif defined(__DJGPP__)
/*
 * Only supports 'off_t' (signed 32-bit) as file size.
 */
int toolx_ftruncate_djgpp(int fd, curl_off_t where)
{
  if(where > INT_MAX)
    return -1;

  return ftruncate(fd, (off_t)where);
}
#endif

#ifdef _WIN32
FILE *tool_execpath(const char *filename, char **pathp)
{
  static char filebuffer[512];
  unsigned long len;
  /* Get the filename of our executable. GetModuleFileName is already declared
   * via inclusions done in setup header file. We assume that we are using
   * the ASCII version here.
   */
  len = GetModuleFileNameA(0, filebuffer, sizeof(filebuffer));
  if(len > 0 && len < sizeof(filebuffer)) {
    /* We got a valid filename - get the directory part */
    char *lastdirchar = strrchr(filebuffer, DIR_CHAR[0]);
    if(lastdirchar) {
      size_t remaining;
      *lastdirchar = 0;
      /* If we have enough space, build the RC filename */
      remaining = sizeof(filebuffer) - strlen(filebuffer);
      if(strlen(filename) < remaining - 1) {
        curl_msnprintf(lastdirchar, remaining, "%s%s", DIR_CHAR, filename);
        *pathp = filebuffer;
        return curlx_fopen(filebuffer, FOPEN_READTEXT);
      }
    }
  }

  return NULL;
}
#endif
