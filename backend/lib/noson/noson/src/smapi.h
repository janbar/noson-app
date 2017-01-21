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
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SMAPI_H
#define SMAPI_H

#include <local_config.h>
#include "smapimetadata.h"
#include "musicservices.h"
#include "sonosplayer.h"
#include "element.h"
#include "private/os/threads/timeout.h"

#include <list>

namespace NSROOT
{

  namespace OS
  {
    class CMutex;
    class CTimeout;
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

    bool AuthTokenExpired() const { return m_authTokenExpired; }

    /**
     * Initialize device link request.
     * @param regUrl The URL for manual registration
     * @return succeeded
     */
    bool GetDeviceLinkCode(std::string& regUrl);

    /**
     * Try to retrieve auth credentials.
     * @param (out) Copy of auth data
     * @return retry (not linked)
     */
    bool GetDeviceAuthToken(SMOAKeyring::OAuth& auth);

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

    bool m_authTokenExpired;
    OS::CTimeout* m_authLinkTimeout;
    std::string m_authLinkCode;
    std::string m_authLinkDeviceId;

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

