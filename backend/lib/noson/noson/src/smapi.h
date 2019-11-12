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

#ifndef SMAPI_H
#define SMAPI_H

#include "local_config.h"
#include "smapimetadata.h"
#include "musicservices.h"
#include "sonossystem.h"
#include "element.h"

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
    SMAPI(const System& system);
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
     * @param searchId The search category
     * @param term Term to search
     * @param index The zero-based index at which to start retrieving metadata items
     * @param count The number of items requested
     * @param metadata (out) Wrapper containing the retrieved content
     * @return succeeded
     */
    bool Search(const std::string& searchId, const std::string& term, int index, int count, SMAPIMetadata& metadata);

    /**
     * Return status of OAuth credentials after SMAPI call failed.
     * @return status of credentials
     */
    bool AuthTokenExpired() const { return m_authTokenExpired; }

    typedef enum {
      Auth_Anonymous  = 0,
      Auth_UserId     = 1,
      Auth_DeviceLink = 2,
      Auth_AppLink    = 3
    } Auth_t;

    Auth_t GetPolicyAuth() const { return m_policyAuth ; }

    const std::string& GetUsername();

    bool GetSessionId(const std::string& user, const std::string& password, SMOAKeyring::Data& auth);

    /**
     * Initialize device link request.
     * @param (out) regUrl The URL for manual registration
     * @return succeeded
     */
    bool GetDeviceLinkCode(std::string& regUrl, std::string& linkCode);

    bool GetAppLink(std::string& regUrl, std::string& linkCode);

    /**
     * Try to retrieve auth credentials after a device link request.
     * While returns 'true', you have to retry call with interval of 5 sec.
     * It will return 'false' if new auth data have been successfully received,
     * or poll interval is reached.
     * @param (out) Fill a copy of received auth data
     * @return retry (not linked)
     */
    bool GetDeviceAuthToken(SMOAKeyring::Data& auth);

    /**
     * Returns the message of fault
     * @return the fault string
     */
    const std::string& GetFaultString() const;

  private:
    OS::CMutex* m_mutex;
    std::string m_language;
    std::string m_deviceSerialNumber;
    std::string m_deviceHouseholdID;
    std::string m_soapHeader;
    std::string m_tz;
    uint32_t m_capabilities;
    Auth_t m_policyAuth;
    
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

    static std::string language(const std::string& locale);
  };
}

#endif /* SMAPI_H */

