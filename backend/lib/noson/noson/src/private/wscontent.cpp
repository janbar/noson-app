/*
 *      Copyright (C) 2014 Jean-Luc Barriere
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "wscontent.h"

#include <cstring>  // for strcmp

using namespace NSROOT;

struct mimetype
{
  const char *content_type;
  const char *extn;
};

static struct mimetype mimetypes[] =
{
  { "",                                   ""    },
  { "application/x-www-form-urlencoded",  ""    },
  { "application/soap+xml",               "xml" },
  { "application/json",                   "txt" },
  { "text/xml",                           "xml" },
  { "text/plain",                         "txt" },
  { "image/gif",                          "gif" },
  { "image/png",                          "png" },
  { "image/jpeg",                         "jpg" },
  { "application/octet-stream",           ""    }
};

CT_t NSROOT::ContentTypeFromMime(const char *mime)
{
  int i;
  for (i = 0; i < CT_UNKNOWN; ++i)
  {
    if (0 == strcmp(mimetypes[i].content_type, mime))
      return (CT_t)i;
  }
  return CT_UNKNOWN;
}

const char* NSROOT::MimeFromContentType(CT_t ct)
{
  if (ct >= 0 && ct < CT_UNKNOWN)
    return mimetypes[ct].content_type;
  return mimetypes[CT_UNKNOWN].content_type;
}

const char* NSROOT::ExtnFromContentType(CT_t ct)
{
  if (ct >= 0 && ct < CT_UNKNOWN)
    return mimetypes[ct].extn;
  return mimetypes[CT_UNKNOWN].extn;
}
