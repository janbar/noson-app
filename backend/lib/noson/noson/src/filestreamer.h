/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef FILESTREAMER_H
#define FILESTREAMER_H

#include "requestbroker.h"
#include "locked.h"

#include <string>
#include <vector>

#define FILESTREAMER_CNAME      "file"
#define FILESTREAMER_PARAM_PATH "path"

namespace NSROOT
{

class FileStreamer : public RequestBroker
{
public:
  FileStreamer();
  ~FileStreamer() override { }
  virtual bool HandleRequest(void* handle, const char* uri) override;

  const char * CommonName() override { return FILESTREAMER_CNAME; }
  RequestBroker::ResourcePtr GetResource(const std::string& title) override;
  RequestBroker::ResourceList GetResourceList() override;
  RequestBroker::ResourcePtr RegisterResource(const std::string& title,
                                              const std::string& description,
                                              const std::string& path,
                                              StreamReader * delegate) override;
  void UnregisterResource(const std::string& uri) override;

private:
  ResourceList m_resources;

  // count current running playback
  LockedNumber<int> m_playbackCount;

  typedef struct {
    const char * codec;
    const char * suffix;
    const char * mime;
  } codec_type;

  static codec_type codecTypeTab[];
  static int codecTypeTabSize;

  enum FileType {
    Mime_flac = 0,
    Mime_mpeg,
  };

  typedef struct {
    const char * mime;
    bool (*probe)(const std::string& filePath);
  } file_type;

  static file_type fileTypeTab[];
  static int fileTypeTabSize;

  static void readParameters(const std::string& streamUrl, std::vector<std::string>& params);
  static std::string getParamValue(const std::vector<std::string>& params, const std::string& name);
  static bool probe(const std::string& filePath, const std::string& mimeType);
  static bool probeFLAC(const std::string& filePath);
  static bool probeMPEG(const std::string& filePath);
  void streamFile(void * handle, const std::string& filePath, const std::string& mimeType);


  void Reply500(void * handle);
  void Reply400(void * handle);
  void Reply429(void * handle);
};

}

#endif /* FILESTREAMER_H */

