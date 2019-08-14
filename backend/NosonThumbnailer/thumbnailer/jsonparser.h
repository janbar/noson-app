/*
 *      Copyright (C) 2014 Jean-Luc Barriere
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef JSONPARSER_H
#define	JSONPARSER_H

#include "sajson.h"

#include <QString>

/**
 * @namespace thumbnailer::JSON
 * @brief This namespace contains all operations to handle JSON content
 */
namespace thumbnailer
{
namespace JSON
{
  class Node
  {
  public:
    Node();
    Node(const sajson::value& value);
    ~Node() { }

    bool IsNull() const;
    bool IsObject() const;
    bool IsArray() const;
    bool IsString() const;
    bool IsDouble() const;
    bool IsInt() const;
    bool IsTrue() const;
    bool IsFalse() const;

    QString GetStringValue() const;
    size_t GetStringSize() const;
    double GetDoubleValue() const;
    int64_t GetBigIntValue() const;
    int32_t GetIntValue() const;

    size_t Size() const;
    Node GetArrayElement(size_t index) const;
    QString GetObjectKey(size_t index) const;
    Node GetObjectValue(size_t index) const;
    Node GetObjectValue(const char *key) const;

  private:
    sajson::value m_value;
  };

  class Document
  {
  public:
    Document(const char* doc);
    ~Document()
    {
      if (m_document)
        delete m_document;
    }

    bool IsValid() const
    {
      return m_isValid;
    }

    Node GetRoot() const;

  private:
    bool m_isValid;
    sajson::document* m_document;
  };
}
}

#endif	/* JSONPARSER_H */
