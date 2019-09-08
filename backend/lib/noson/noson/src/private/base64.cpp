/*
 *  Credits to polfosol
 */

#include "base64.h"

#include <string.h>

using namespace NSROOT;

const char* Base64::B64chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

const int Base64::B64index[256] =
{
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
  0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

size_t Base64::b64encode(const void * data, size_t len, char ** b64)
{
  size_t b64len = (len + 2) / 3 * 4;
  char * r = new char [b64len];
  memset(r, '=', b64len);

  char * p = (char*) data;
  size_t j = 0, pad = len % 3;
  const size_t last = len - pad;

  for (size_t i = 0; i < last; i += 3)
  {
    int n = int(p[i]) << 16 | int(p[i + 1]) << 8 | p[i + 2];
    r[j++] = B64chars[n >> 18];
    r[j++] = B64chars[n >> 12 & 0x3F];
    r[j++] = B64chars[n >> 6 & 0x3F];
    r[j++] = B64chars[n & 0x3F];
  }
  if (pad)  /// set padding
  {
    int n = --pad ? int(p[last]) << 8 | p[last + 1] : p[last];
    r[j++] = B64chars[pad ? n >> 10 & 0x3F : n >> 2];
    r[j++] = B64chars[pad ? n >> 4 & 0x03F : n << 4 & 0x3F];
    r[j++] = pad ? B64chars[n << 2 & 0x3F] : '=';
  }
  *b64 = r;
  return b64len;
}

size_t Base64::b64decode(const void * b64, size_t len, char ** data)
{
  if (len == 0) return 0;

  unsigned char *p = (unsigned char*) b64;
  size_t j = 0,
      pad1 = len % 4 || p[len - 1] == '=',
      pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
  const size_t last = (len - pad1) / 4 << 2;
  size_t datalen = last / 4 * 3 + pad1 + pad2;
  char * r = new char [datalen];
  memset(r, '\0', datalen);

  for (size_t i = 0; i < last; i += 4)
  {
    int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
    r[j++] = n >> 16;
    r[j++] = n >> 8 & 0xFF;
    r[j++] = n & 0xFF;
  }
  if (pad1)
  {
    int n = B64index[p[last]] << 18 | B64index[p[last + 1]] << 12;
    r[j++] = n >> 16;
    if (pad2)
    {
      n |= B64index[p[last + 2]] << 6;
      r[j++] = n >> 8 & 0xFF;
    }
  }
  *data = r;
  return datalen;
}
