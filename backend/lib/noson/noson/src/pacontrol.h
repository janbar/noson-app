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

#ifndef PACONTROL_H
#define PACONTROL_H

#include <string>
#include <list>
#include <pulse/pulseaudio.h>

#include "local_config.h"

namespace NSROOT
{

class PAControl
{
public:

  struct Source
  {
    unsigned index;
    std::string name;
    std::string description;
    std::string monitorOfSinkName;
  };

  typedef std::list<Source> SourceList;

  struct Sink
  {
    unsigned index;
    unsigned ownerModule;
    std::string name;
    std::string description;
    std::string monitorSourceName;
  };

  typedef std::list<Sink> SinkList;

  PAControl(const std::string& ctxname);
  virtual ~PAControl();

  bool connect();
  void disconnect();
  bool getSourceList(SourceList * deviceList);
  bool getSinkList(SinkList * deviceList);

  unsigned newSink(const char * sinkName, const char * description);
  void deleteSink(unsigned index);

private:
  // Prevent copy
  PAControl(const PAControl& other);
  PAControl& operator=(const PAControl& other);

  static void pa_state_cb(pa_context * c, void * h);
  static void pa_sourcelist_cb(pa_context * c, const pa_source_info * l, int eol, void * h);
  static void pa_sinklist_cb(pa_context * c, const pa_sink_info * l, int eol, void * h);
  static void pa_contextindex_cb(pa_context * c, unsigned index, void * h);
  static void pa_contextsuccess_cb(pa_context * c, int success, void * h);

  std::string m_name;
  pa_mainloop *m_pa_ml;
  pa_mainloop_api *m_pa_mlapi;
  pa_context *m_pa_ctx;
  pa_context_state_t m_state;
};

}

#endif /* PACONTROL_H */

