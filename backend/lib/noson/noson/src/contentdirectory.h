/*
 *      Copyright (C) 2014-2015 Jean-Luc Barriere
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

#ifndef CONTENTDIRECTORY_H
#define CONTENTDIRECTORY_H

#include <local_config.h>
#include "service.h"
#include "digitalitem.h"
#include "eventhandler.h"
#include "subscription.h"
#include "locked.h"

#include <list>
#include <vector>
#include <stdint.h>

#define BROWSE_COUNT  100

namespace NSROOT
{
  class Subscription;

  class ContentDirectory : public Service, public EventSubscriber
  {
  public:
    ContentDirectory(const std::string& serviceHost, unsigned servicePort);
    ContentDirectory(const std::string& serviceHost, unsigned servicePort, EventHandler& eventHandler, Subscription& subscription, void* CBHandle = 0, EventCB eventCB = 0);
    ~ContentDirectory();

    static const std::string Name;
    static const std::string ControlURL;
    static const std::string EventURL;
    static const std::string SCPDURL;

    const std::string& GetName() const { return Name; }

    const std::string& GetControlURL() const { return ControlURL; }

    const std::string& GetEventURL() const { return EventURL; }

    const std::string& GetSCPDURL() const { return SCPDURL; }

    bool Browse(const std::string& objectId, unsigned index, unsigned count, ElementList& vars);

    bool RefreshShareIndex();

    bool DestroyObject(const std::string& objectID);

    // Implements EventSubscriber
    virtual void HandleEventMessage(EventMessagePtr msg);

    Locked<ContentProperty>& GetContentProperty() { return m_property; }

  private:
    EventHandler m_eventHandler;
    Subscription m_subscription;
    void* m_CBHandle;
    EventCB m_eventCB;

    Locked<ContentProperty> m_property;
  };

  /////////////////////////////////////////////////////////////////////////////
  ////
  //// ContentSearch
  ////

  typedef enum
  {
    SearchArtist,
    SearchAlbum,
    SearchGenre,
    SearchTrack,
    SearchComposer,
    SearchContributor,
    SearchPlaylist,
    SearchRadio,
    SearchQueue,
    SearchSonosPlaylist,
    SearchShare,
    SearchCategory,
  } Search_t;

  class ContentSearch
  {
  public:
    ContentSearch(Search_t search, const std::string& string) : m_search(search), m_string(string) {}
    virtual ~ContentSearch() {}

    virtual std::string Root() const;

  protected:
    Search_t m_search;
    std::string m_string;
  };

  /////////////////////////////////////////////////////////////////////////////
  ////
  //// ContentList
  ////

  class ContentList
  {
    typedef std::list<DigitalItemPtr> List;

    friend class iterator;
  public:
    ContentList(ContentDirectory& service, const ContentSearch& search, unsigned bulksize = BROWSE_COUNT);
    ContentList(ContentDirectory& service, const std::string& objectID, unsigned bulksize = BROWSE_COUNT);
    virtual ~ContentList() {}

    class iterator
    {
      friend class ContentList;
    public:
      typedef iterator self_type;
      iterator() : c(0) {}
      virtual ~iterator() {}
      self_type operator++() { self_type i0 = *this; if (c) c->Next(i); return i0; }
      self_type& operator++(int junk) { (void)junk; if (c) c->Next(i); return *this; }
      self_type operator--() { self_type i0 = *this; if (c) c->Previous(i); return i0; }
      self_type& operator--(int junk) { (void)junk; if (c) c->Previous(i); return *this; }
      bool operator==(const self_type& rhs) const { return rhs.i == i; }
      bool operator!=(const self_type& rhs) const { return rhs.i != i; }
      List::value_type& operator*() const { return *i; }
      List::value_type* operator->() const { return &*i; }
    private:
      ContentList* c;
      List::iterator i;
      iterator(ContentList* _c, const List::iterator& _i) : c(_c) { if (_c) i = _i; }
    };

    bool failure() const { return !m_succeeded; }

    iterator begin() { return iterator(this, m_list.begin()); }

    iterator end() { return iterator(this, m_list.end()); }

    unsigned size() { return m_totalCount; }

    unsigned GetUpdateID() { return m_baseUpdateID; }

  private:
    bool m_succeeded;
    ContentDirectory& m_service;
    unsigned m_bulkSize;
    std::string m_root;
    unsigned m_baseUpdateID;
    unsigned m_totalCount;
    unsigned m_browsedCount;
    unsigned m_lastUpdateID;

    List m_list;

    bool Next(List::iterator& i);
    bool Previous(List::iterator& i);
    bool BrowseContent(unsigned startingIndex, unsigned count, List::iterator position);
  };

  /////////////////////////////////////////////////////////////////////////////
  ////
  //// ContentBrowser
  ////

  class ContentBrowser
  {
  public:
    typedef std::vector<DigitalItemPtr> Table;

    ContentBrowser(ContentDirectory& service, const ContentSearch& search, unsigned count = BROWSE_COUNT);
    ContentBrowser(ContentDirectory& service, const std::string& objectID, unsigned count = BROWSE_COUNT);
    virtual ~ContentBrowser() {}

    bool Browse(unsigned startingIndex, unsigned count);

    unsigned index() { return m_startingIndex; }

    unsigned count() { return (unsigned) m_table.size(); }

    unsigned total() { return m_totalCount; }

    Table& table() { return m_table; }

    unsigned GetUpdateID() { return m_baseUpdateID; }

  private:
    ContentDirectory& m_service;
    std::string m_root;
    unsigned m_baseUpdateID;
    unsigned m_totalCount;
    unsigned m_startingIndex;
    unsigned m_lastUpdateID;

    Table m_table;

    bool BrowseContent(unsigned startingIndex, unsigned count, Table::iterator position);
  };

}

#endif	/* AVTRANSPORT_H */
