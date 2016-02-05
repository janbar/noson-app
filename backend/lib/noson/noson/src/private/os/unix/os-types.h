#pragma once

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

typedef int net_socket_t;
#define INVALID_SOCKET_VALUE        (-1)

typedef long LONG;
typedef LONG HRESULT;

#define _FILE_OFFSET_BITS 64
#define FILE_BEGIN        SEEK_SET
#define FILE_CURRENT      SEEK_CUR
#define FILE_END          SEEK_END

// Success codes
#define S_OK          0L
#define S_FALSE       1L
#define FAILED(hr)    (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

// Error codes
#define ERROR_FILENAME_EXCED_RANGE 206L
#define ERROR_INVALID_NAME         123L
#define E_OUTOFMEMORY              0x8007000EL
#define E_FAIL                     0x8004005EL

#if defined(__linux__)
#include <limits.h>
#define MAX_PATH PATH_MAX
#elif defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/cdefs.h>
#include <sys/syslimits.h>
#define MAX_PATH PATH_MAX
#else
#define MAX_PATH 256
#endif

#if defined(__APPLE__)
#include <stdio.h> /* for fpos_t */
#include <sched.h>
#include <AvailabilityMacros.h>
typedef int64_t   off64_t;
typedef off_t     __off_t;
typedef off64_t   __off64_t;
typedef fpos_t    fpos64_t;
#define __stat64  stat
#define stat64    stat
#define statfs64  statfs
#define fstat64   fstat
#elif defined(__FreeBSD__)
#include <stdio.h> /* for fpos_t */
typedef int64_t   off64_t;
typedef off_t     __off_t;
typedef off64_t   __off64_t;
typedef fpos_t    fpos64_t;
#define __stat64  stat
#define stat64    stat
#define statfs64  statfs
#define fstat64   fstat
#else
#define __stat64  stat64
#endif

#include <string.h>
#define strnicmp(X,Y,N) strncasecmp(X,Y,N)

typedef unsigned char byte;

/* OS dependent path separator */
#ifndef PATH_SEPARATOR_CHAR
#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_STRING "/"
#endif
