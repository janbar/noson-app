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
#include "pulsestreamer.h"
#include "pacontrol.h"
#include "pasource.h"
#include "audiostream.h"
#include "flacencoder.h"
#include "private/debug.h"
#include "requestbroker.h"
#include "private/socket.h"

#include <cstring>

#define PULSESTREAMER_URI       "/music/track.flac?id=pulse"
#define PULSESTREAMER_ICON_URI  "/images/pulseaudio.png"
#define PULSESTREAMER_CONTENT   "audio/flac"
#define PULSESTREAMER_DESC      "Audio stream from %s"
#define PULSESTREAMER_TIMEOUT   10000
#define PULSESTREAMER_MAX_PB    3
#define PA_SINK_NAME            "noson"
#define PA_CLIENT_NAME          PA_SINK_NAME

using namespace NSROOT;

PulseStreamer::PulseStreamer()
: SONOS::RequestBroker()
, m_resources()
, m_sinkIndex(0)
, m_playbackCount(0)
{
  ResourcePtr ptr = ResourcePtr(new Resource());
  ptr->title = PULSESTREAMER_CNAME;
  ptr->uri = PULSESTREAMER_URI;
  ptr->contentType = PULSESTREAMER_CONTENT;
  ptr->iconUri = PULSESTREAMER_ICON_URI;
  ptr->description = PULSESTREAMER_DESC;
  m_resources.push_back(ptr);
}

bool PulseStreamer::HandleRequest(void* handle, const char* uri)
{
  std::string requrl(uri);
  if (!IsAborted() && requrl.compare(0, strlen(PULSESTREAMER_URI), PULSESTREAMER_URI) == 0)
  {
    streamSink(handle);
    return true;
  }
  return false;
}

RequestBroker::ResourcePtr PulseStreamer::GetResource(const std::string& title)
{
  return m_resources.front();
}

RequestBroker::ResourceList PulseStreamer::GetResourceList()
{
  return m_resources;
}

RequestBroker::ResourcePtr PulseStreamer::RegisterResource(const std::string& sourceUrl)
{
  (void)sourceUrl;
  return ResourcePtr();
}

void PulseStreamer::UnregisterResource(const std::string& uri)
{
  (void)uri;
}

std::string PulseStreamer::GetPASink()
{
  std::string deviceName;
  SONOS::PAControl::SinkList sinks;
  SONOS::PAControl pacontrol(PA_CLIENT_NAME);
  if (pacontrol.connect())
  {
    bool cont = true;
    for (;;)
    {
      pacontrol.getSinkList(&sinks);
      for (SONOS::PAControl::Sink ad : sinks)
      {
        if (ad.name == PA_SINK_NAME)
        {
          SONOS::DBG(DBG_DEBUG, "%s: Found device %d: %s\n", __FUNCTION__, ad.index, ad.monitorSourceName.c_str());
          deviceName = ad.monitorSourceName;
          m_sinkIndex.Store(ad.ownerModule); // own the module
          break;
        }
      }
      if (!deviceName.empty() || !cont)
        break;
      DBG(DBG_DEBUG, "%s: create sink (%s)\n", __FUNCTION__, PA_SINK_NAME);
      m_sinkIndex.Store(pacontrol.newSink(PA_SINK_NAME, PA_SINK_NAME));
      cont = false;
    }
    pacontrol.disconnect();
  }
  return deviceName;
}

void PulseStreamer::FreePASink()
{
  // Lock count
  // and check if an other playback is running before delete the sink
  SONOS::LockedNumber<int>::pointer p = m_playbackCount.Get();
  if (*p == 1 && m_sinkIndex.Load())
  {
    SONOS::PAControl pacontrol(PA_CLIENT_NAME);
    if (pacontrol.connect())
    {
      DBG(DBG_DEBUG, "%s: delete sink (%s)\n", __FUNCTION__, PA_SINK_NAME);
      pacontrol.deleteSink(m_sinkIndex.Load());
      pacontrol.disconnect();
    }
    m_sinkIndex.Store(0);
  }
}

void PulseStreamer::streamSink(void * handle)
{
  m_playbackCount.Add(1);

  std::string deviceName = GetPASink();

  if (deviceName.empty())
    Reply500(handle);
  else if (m_playbackCount.Load() > PULSESTREAMER_MAX_PB)
    Reply429(handle);
  else
  {
    SONOS::AudioSource * src = new SONOS::PASource(PA_CLIENT_NAME, deviceName);
    SONOS::AudioEncoder * enc = new SONOS::FLACEncoder();
    SONOS::AudioStream ai(*src, *enc);
    ai.start();

    std::string resp;
    resp.assign("HTTP/1.1 200 OK\r\n")
        .append("Content-type: audio/flac\r\n")
        .append("Transfer-encoding: chunked\r\n")
        .append("Server: Linux UPnP/1.0 Noson/1.0 (ACR_noson)\r\n")
        .append("Connection: close\r\n")
        .append("\r\n");

    if (Reply(handle, resp.c_str(), resp.length()))
    {
      char buf[4008];
      int r = 0;
      while (!IsAborted() && (r = ai.read(buf + 5, 4000, PULSESTREAMER_TIMEOUT)) > 0)
      {
        char str[6];
        snprintf(str, 6, "%.3x\r\n", (unsigned)r);
        memcpy(buf, str, 5);
        memcpy(buf + 5 + r, "\r\n", 2);
        if (!Reply(handle, buf, r + 7))
          break;
      }
      if (r == 0)
        Reply(handle, "0\r\n\r\n", 5);
    }

    ai.stop();
    delete enc;
    delete src;
  }

  FreePASink();
  m_playbackCount.Sub(1);
}

void PulseStreamer::Reply500(void* handle)
{
  std::string resp;
  resp.assign("HTTP/1.1 500 Internal Server Error\r\n")
      .append("Server: Linux UPnP/1.0 Noson/1.0 (ACR_noson)\r\n")
      .append("Connection: close\r\n")
      .append("\r\n");
  Reply(handle, resp.c_str(), resp.length());
}

void PulseStreamer::Reply400(void* handle)
{
  std::string resp;
  resp.append("HTTP/1.1 400 Bad Request\r\n")
      .append("Server: Linux UPnP/1.0 Noson/1.0 (ACR_noson)\r\n")
      .append("Connection: close\r\n")
      .append("\r\n");
  Reply(handle, resp.c_str(), resp.length());
}

void PulseStreamer::Reply429(void* handle)
{
  std::string resp;
  resp.append("HTTP/1.1 429 Too Many Requests\r\n")
      .append("Server: Linux UPnP/1.0 Noson/1.0 (ACR_noson)\r\n")
      .append("Connection: close\r\n")
      .append("\r\n");
  Reply(handle, resp.c_str(), resp.length());
}
