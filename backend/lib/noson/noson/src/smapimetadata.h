/*
 *      Copyright (C) 2016 Jean-Luc Barriere
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

#ifndef SMAPIMETADATA_H
#define SMAPIMETADATA_H

#include <local_config.h>
#include "element.h"
#include "digitalitem.h"
#include "musicservices.h"

namespace NSROOT
{
  struct SMAPIItem
  {
    enum DisplayType { Grid = 0, List = 1, Hero, Editorial } displayType;
    DigitalItemPtr item;
    DigitalItemPtr uriMetadata;
  };

  typedef std::vector<SMAPIItem> SMAPIItemList;

  class SMAPIMetadata
  {
  public:
    SMAPIMetadata();
    SMAPIMetadata(const SMServicePtr& svc, const std::string& xml, const std::string& root);
    virtual ~SMAPIMetadata() {}

    void Reset(const SMServicePtr& svc, const std::string& xml, const std::string& root);

    bool IsValid() const { return m_valid; }
    unsigned StartIndex() const { return m_startIndex; }
    unsigned ItemCount() const { return m_itemCount; }
    unsigned TotalCount() const { return m_totalCount; }

    SMAPIItemList GetItems();

    const ElementList& GetElements() const { return m_list; }

  private:
    uint32_t m_startIndex;
    uint32_t m_itemCount;
    uint32_t m_totalCount;
    bool m_valid;
    ElementList m_list;
    std::string m_root;
    SMServicePtr m_service;

    bool ParseMessage(const std::string& data);

    ElementPtr ParseMediaCollection(const void*);
    ElementPtr ParseMediaMetadata(const void*);
    bool ParseStreamMetadata(const void*, Element&);
    bool ParseTrackMetadata(const void*, Element&);
  };
}

#endif /* SMAPIMETADATA_H */

