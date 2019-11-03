/*
 *      Copyright (C) 2014-2016 Jean-Luc Barriere
 *
 *  This file is part of Noson
 *
 *  Noson is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Noson is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DIGITALITEM_H
#define	DIGITALITEM_H

#include "local_config.h"
#include "element.h"
#include "sharedptr.h"

namespace NSROOT
{

  class DigitalItem;

  typedef SHARED_PTR<DigitalItem> DigitalItemPtr;
  typedef std::vector<DigitalItemPtr> DigitalItemList;

  class DigitalItem
  {
  public:
    typedef enum
    {
      Type_container  = 0,
      Type_item       = 1,
      Type_unknown,
    } Type_t;

    typedef enum {
      // container
      SubType_playlistContainer = 0,
      SubType_album             = 1,
      SubType_genre,
      SubType_person,
      SubType_channelGroup,
      SubType_epgContainer,
      SubType_storageSystem,
      SubType_storageVolume,
      SubType_storageFolder,
      SubType_bookmarkFolder,
      // item
      SubType_audioItem,
      SubType_videoItem,
      SubType_imageItem,
      SubType_playlistItem,
      SubType_textItem,
      SubType_bookmarkItem,
      SubType_epgItem,
      // else
      SubType_unknown,
    } SubType_t;

    DigitalItem();
    DigitalItem(Type_t _type, SubType_t _subType = SubType_unknown);
    DigitalItem(const std::string& objectID, const std::string& parentID, bool restricted, const ElementList& vars);
    virtual ~DigitalItem() {}

    bool IsValid() const { return m_type != Type_unknown; }

    bool IsItem() const { return m_type == Type_item; }

    bool IsContainer() const { return m_type == Type_container; }

    SubType_t subType() const { return m_subType; }

    const std::string& GetObjectID() const { return m_objectID; }

    void SetObjectID(const std::string& val) { m_objectID = val; }

    const std::string& GetParentID() const { return m_parentID; }

    void SetParentID(const std::string& val) { m_parentID = val; }

    bool GetRestricted() const { return m_restricted; }

    void SetRestricted(bool val) { m_restricted = val; }

    const std::string& GetValue(const std::string& key) const { return m_vars.GetValue(key); }

    const ElementPtr GetProperty(const std::string& key) const;

    std::vector<ElementPtr> GetCollection(const std::string& key) const;

    const ElementPtr& SetProperty(const ElementPtr& var);

    const ElementPtr& SetProperty(const Element& var) { return SetProperty(ElementPtr(new Element(var))); }

    const ElementPtr& SetProperty(const std::string& key, const std::string& value) { return SetProperty(Element(key, value)); }

    void RemoveProperty(const std::string& key);

    std::string DIDL() const;

    void Clone(DigitalItem& _item) const;

  private:
    Type_t m_type;
    SubType_t m_subType;

    bool m_restricted;
    std::string m_objectID;
    std::string m_parentID;
    ElementList m_vars;

    static const char* TypeTable[Type_unknown + 1];
    static const char* SubTypeTable[SubType_unknown + 1];

    // prevent copy
    DigitalItem(const DigitalItem&);
    DigitalItem& operator=(const DigitalItem&);
  };
}

#endif	/* DIGITALITEM_H */

