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

#ifndef MUSICSERVICES_H
#define MUSICSERVICES_H

#include <local_config.h>
#include "service.h"

#include <list>

namespace NSROOT
{

  class MusicServices : public Service
  {
  public:
    MusicServices(const std::string& serviceHost, unsigned servicePort);
    ~MusicServices() {}

    static const std::string Name;
    static const std::string ControlURL;
    static const std::string EventURL;
    static const std::string SCPDURL;

    const std::string& GetName() const { return Name; }

    const std::string& GetControlURL() const { return ControlURL; }

    const std::string& GetEventURL() const { return EventURL; }

    const std::string& GetSCPDURL() const { return SCPDURL; }

    bool ListAvailableServices(ElementList& vars);
  };


  /////////////////////////////////////////////////////////////////////////////
  ////
  //// MusicServiceList
  ////

  class SMService : protected ElementList
  {
  public:
    SMService() : ElementList() { }

    friend class MusicServiceList;

    const std::string& GetId() const;
    const std::string& GetName() const;
    const std::string& GetVersion() const;
    const std::string& GetUri() const;
    const std::string& GetSecureUri() const;
    const std::string& GetContainerType() const;
    const std::string& GetCapabilities() const;
    ElementPtr GetPolicy() const;
    ElementPtr GetStrings() const;
    ElementPtr GetPresentationMap() const;

    std::string GetServiceType() const;
    const std::string& GetAttribut(const std::string& key) const { return this->GetValue(key); }
  };

  typedef shared_ptr<SMService> SMServicePtr;

  class MusicServiceList
  {
    typedef std::list<SMServicePtr> List;

    friend class iterator;
  public:
    MusicServiceList(MusicServices& service);
    virtual ~MusicServiceList() {}

    class iterator
    {
      friend class MusicServiceList;
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
      MusicServiceList* c;
      List::iterator i;
      iterator(MusicServiceList* _c, const List::iterator& _i) : c(_c) { if (_c) i = _i; }
    };

    bool failure() const { return !m_succeeded; }

    iterator begin() { return iterator(this, m_list.begin()); }

    iterator end() { return iterator(this, m_list.end()); }

    unsigned size() { return m_totalCount; }

  private:
    bool m_succeeded;
    MusicServices& m_service;
    unsigned m_totalCount;

    List m_list;

    bool Next(List::iterator& i);
    bool Previous(List::iterator& i);
    bool Browse();
    bool ParseAvailableServiceDescriptorList(const std::string& xml);
  };
}

#endif /* MUSICSERVICES_H */

