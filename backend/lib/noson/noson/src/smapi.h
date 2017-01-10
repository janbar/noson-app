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

#ifndef SMAPI_H
#define SMAPI_H

#include <local_config.h>
#include "smapimetadata.h"
#include "musicservices.h"
#include "sonosplayer.h"
#include "element.h"

#include <list>

namespace NSROOT
{

  namespace OS
  {
    class CMutex;
  };

  class URIParser;
  
  class SMAPI
  {
  public:
    SMAPI(const PlayerPtr& player);
    virtual ~SMAPI();

    bool Init(const SMServicePtr& smsvc, const std::string& locale);
    
    const ElementList& AvailableSearchCategories() const { return m_searchCategories; }

    /**
     * Load metadata for a container or item.
     * @param id Unique ID of the item to browse
     * @param index The zero-based index at which to start retrieving metadata items
     * @param count The number of items requested
     * @param recursive If true, returns a flat collection of track metadata
     * @param metadata (out) Wrapper containing the retrieved content
     * @return succeeded
     */
    bool GetMetadata(const std::string& id, int index, int count, bool recursive, SMAPIMetadata& metadata);

    /**
     * Load metadata for a media item.
     * @param id Unique ID of the item to load
     * @param metadata (out) Wrapper containing the retrieved content
     * @return succeeded
     */
    bool GetMediaMetadata(const std::string& id, SMAPIMetadata& metadata);

    /**
     * Search metadata for items in a category.
     * @param searchId
     * @param term
     * @param index
     * @param count
     * @param metadata
     * @return
     */
    bool Search(const std::string& searchId, const std::string& term, int index, int count, SMAPIMetadata& metadata);

  private:
    OS::CMutex* m_mutex;
    PlayerPtr m_player;
    std::string m_locale;
    std::string m_deviceSerialNumber;
    std::string m_deviceHouseholdID;
    std::string m_soapHeader;
    
    SMServicePtr m_service;
    ElementList m_searchCategories;
    std::list<std::pair<ElementPtr, ElementList> > m_presentation;
    URIParser* m_uri;
    bool m_valid;

    bool parsePresentationMap(const std::string& xml);

    bool makeSoapHeader();

    ElementList DoCall(const std::string& action, const ElementList& args);

    ElementList m_fault;

    void SetFault(const ElementList& vars);

    ElementList Request(const std::string& action, const ElementList& args);

    // prevent copy
    SMAPI(const SMAPI&);
    SMAPI& operator=(const SMAPI&);
  };
}

#endif /* SMAPI_H */

