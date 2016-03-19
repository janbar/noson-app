#pragma once

#if !defined(__WINDOWS__)
#define __WINDOWS__
#endif

/* Enable LEAN_AND_MEAN support */
#define WIN32_LEAN_AND_MEAN

/* Don't define min() and max() to prevent a clash with std::min() and std::max */
#ifndef NOMINMAX
#define NOMINMAX
#endif

/* Disable warning C4005: '_WINSOCKAPI_' : macro redefinition */
#pragma warning(disable:4005)
#include <winsock2.h>
#pragma warning(default:4005)
#include <windows.h>
#include <wchar.h>
#include <time.h>
#include <sys/timeb.h>
#include <io.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

/* prevent inclusion of wingdi.h */
#define NOGDI

typedef SOCKET net_socket_t;
#define INVALID_SOCKET_VALUE        INVALID_SOCKET

/* OS dependent path separator */
#ifndef PATH_SEPARATOR_CHAR
#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_STRING "\\"
#endif

#ifndef _SSIZE_T_DEFINED
#ifdef  _WIN64
typedef __int64    ssize_t;
#else
typedef _W64 int   ssize_t;
#endif
#define _SSIZE_T_DEFINED
#endif

__inline int usleep(unsigned int usec)
{
  Sleep((DWORD)(usec / 1000));
  return 0;
}

__inline unsigned int sleep(unsigned int sec)
{
  Sleep((DWORD)(sec * 1000));
  return 0;
}

/* Using MS Visual C++ compilers */
#if defined(_MSC_VER)
#include "msc_inttypes.h"

struct timezone
{
  int	tz_minuteswest;
  int	tz_dsttime;
};

/* String to 64-bit int */
#if (_MSC_VER < 1800)
#define atoll(S) _atoi64(S)
#endif

/* Prevent deprecation warnings */
#if (_MSC_VER < 1900)
#define snprintf _snprintf
#endif

#define strnicmp _strnicmp

#else
#include <inttypes.h>
#endif /* _MSC_VER */
