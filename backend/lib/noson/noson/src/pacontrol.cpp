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
#include "pacontrol.h"
#include "private/debug.h"

#include <cassert>

using namespace NSROOT;

PAControl::PAControl(const std::string& ctxname)
: m_name(ctxname)
, m_pa_ml(nullptr)
, m_pa_mlapi(nullptr)
, m_pa_ctx(nullptr)
, m_state(PA_CONTEXT_UNCONNECTED)
{
}

PAControl::~PAControl()
{
  disconnect();
}

bool PAControl::connect()
{
  if (m_pa_ctx)
    return true;

  // Create a mainloop API and connection variables
  m_pa_ml = pa_mainloop_new();
  m_pa_mlapi = pa_mainloop_get_api(m_pa_ml);
  m_pa_ctx = pa_context_new(m_pa_mlapi, m_name.c_str());

  // Connect to the pulse server
  pa_context_connect(m_pa_ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

  // Defines a callback so the server will tell us it's state.
  // Our callback will wait for the state to be ready.  The callback will
  // modify the variable to 1 so we know when we have a connection and it's
  // ready. If there's an error, the callback will set pa_ready to 2
  pa_context_set_state_callback(m_pa_ctx, &PAControl::pa_state_cb, this);

  // Now we'll enter into an infinite loop until we get the data we receive
  // or if there's an error
  for (;;)
  {
    // We can't do anything until PA is ready or has failed, so just iterate
    // the mainloop and continue
    switch (m_state)
    {
      // There are just here for reference
    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
    default:
      break;
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
      disconnect();
      return false;
    case PA_CONTEXT_READY:
      // At this point, we're connected to the server and ready to make
      // requests
      return true;
    }
    pa_mainloop_iterate(m_pa_ml, 1, NULL);
  }
}

void PAControl::disconnect()
{
  if (m_pa_ctx)
  {
    pa_context_disconnect(m_pa_ctx);
    pa_context_unref(m_pa_ctx);
    pa_mainloop_free(m_pa_ml);
    m_pa_ctx = nullptr;
    m_pa_mlapi = nullptr;
    m_pa_ml = nullptr;
  }
  m_state = PA_CONTEXT_UNCONNECTED;
}


bool PAControl::getSourceList(SourceList * deviceList)
{
  assert(deviceList);
  deviceList->clear();
  if (m_state != PA_CONTEXT_READY)
    return false;

  // We'll need these state variables to keep track of our request
  pa_operation_state state;
  pa_operation * pa_op;

  // This sends an operation to the server. cb is our callback function and
  // data will be passed to the callback. The operation id is stored in the
  // pa_op variable
  pa_op = pa_context_get_source_info_list(m_pa_ctx,
              &PAControl::pa_sourcelist_cb,
              deviceList
              );
  // Now we'll enter into an infinite loop until we get the data we receive
  // or if there's an error
  while ((state = pa_operation_get_state(pa_op)) == PA_OPERATION_RUNNING)
  {
    // Iterate the main loop and go again.  The second argument is whether
    // or not the iteration should block until something is ready to be
    // done.  Set it to zero for non-blocking.
    pa_mainloop_iterate(m_pa_ml, 1, NULL);
  }
  pa_operation_unref(pa_op);
  return (state == PA_OPERATION_DONE);
}

bool PAControl::getSinkList(SinkList * deviceList)
{
  assert(deviceList);
  deviceList->clear();
  if (m_state != PA_CONTEXT_READY)
    return false;

  // We'll need these state variables to keep track of our request
  pa_operation_state state;
  pa_operation * pa_op;

  // This sends an operation to the server. cb is our callback function and
  // data will be passed to the callback. The operation id is stored in the
  // pa_op variable
  pa_op = pa_context_get_sink_info_list(m_pa_ctx,
              &PAControl::pa_sinklist_cb,
              deviceList
              );
  // Now we'll enter into an infinite loop until we get the data we receive
  // or if there's an error
  while ((state = pa_operation_get_state(pa_op)) == PA_OPERATION_RUNNING)
  {
    // Iterate the main loop and go again.  The second argument is whether
    // or not the iteration should block until something is ready to be
    // done.  Set it to zero for non-blocking.
    pa_mainloop_iterate(m_pa_ml, 1, NULL);
  }
  pa_operation_unref(pa_op);
  return (state == PA_OPERATION_DONE);
}

unsigned PAControl::newSink(const char * sinkName, const char * description)
{
  if (m_state != PA_CONTEXT_READY)
    return false;

  // We'll need these state variables to keep track of our request
  pa_operation_state state;
  pa_operation * pa_op;
  unsigned index;

  std::string args;
  args.assign("sink_name=").append(sinkName);
  if (*description != '\0')
    args.append(" sink_properties=device.description=\"").append(description).append("\"");

  // This sends an operation to the server. cb is our callback function and
  // data will be passed to the callback. The operation id is stored in the
  // pa_op variable
  pa_op = pa_context_load_module(m_pa_ctx,
              "module-null-sink",
              args.c_str(),
              &PAControl::pa_contextindex_cb,
              &index
              );
  // Now we'll enter into an infinite loop until we get the data we receive
  // or if there's an error
  while ((state = pa_operation_get_state(pa_op)) == PA_OPERATION_RUNNING)
  {
    // Iterate the main loop and go again.  The second argument is whether
    // or not the iteration should block until something is ready to be
    // done.  Set it to zero for non-blocking.
    pa_mainloop_iterate(m_pa_ml, 1, NULL);
  }
  pa_operation_unref(pa_op);
  if (state == PA_OPERATION_DONE && index != PA_INVALID_INDEX)
  {
    DBG(DBG_DEBUG, "%s: create succeeded (%u)\n", __FUNCTION__, index);
    return index;
  }
  DBG(DBG_ERROR, "%s: create failed\n", __FUNCTION__);
  return PA_INVALID_INDEX;
}

void PAControl::deleteSink(unsigned index)
{
  if (m_state != PA_CONTEXT_READY || index == PA_INVALID_INDEX)
    return;

  // We'll need these state variables to keep track of our request
  pa_operation_state state;
  pa_operation * pa_op;

  // This sends an operation to the server. cb is our callback function and
  // data will be passed to the callback. The operation id is stored in the
  // pa_op variable
  pa_op = pa_context_unload_module(m_pa_ctx,
              index,
              &PAControl::pa_contextsuccess_cb,
              0
              );
  // Now we'll enter into an infinite loop until we get the data we receive
  // or if there's an error
  while ((state = pa_operation_get_state(pa_op)) == PA_OPERATION_RUNNING)
  {
    // Iterate the main loop and go again.  The second argument is whether
    // or not the iteration should block until something is ready to be
    // done.  Set it to zero for non-blocking.
    pa_mainloop_iterate(m_pa_ml, 1, NULL);
  }
  if (state == PA_OPERATION_DONE)
    DBG(DBG_DEBUG, "%s: delete succeeded (%u)\n", __FUNCTION__, index);
  else
    DBG(DBG_ERROR, "%s: delete failed (%u)\n", __FUNCTION__, index);
  pa_operation_unref(pa_op);
}

// This callback gets called when our context changes state
void PAControl::pa_state_cb(pa_context * c, void * h)
{
  PAControl * handle = static_cast<PAControl*> (h);
  handle->m_state = pa_context_get_state(c);
}

// pa_mainloop will call this function when it's ready to tell us about a source.
// Since we're not threading, there's no need for mutexes on the device list
// structure
void PAControl::pa_sourcelist_cb(pa_context * c, const pa_source_info * l, int eol, void * h)
{
  if (eol > 0)
    return;
 
  SourceList * handle = static_cast<SourceList*>(h);

  Source ad;
  ad.index = l->index;
  ad.name.assign(l->name);
  ad.description.assign(l->description);
  ad.monitorOfSinkName.assign(l->monitor_of_sink_name);

  handle->push_back(ad);
}

// pa_mainloop will call this function when it's ready to tell us about a sink.
// Since we're not threading, there's no need for mutexes on the device list
// structure
void PAControl::pa_sinklist_cb(pa_context * c, const pa_sink_info * l, int eol, void * h)
{
  if (eol > 0)
    return;

  SinkList * handle = static_cast<SinkList*>(h);

  Sink ad;
  ad.index = l->index;
  ad.ownerModule = l->owner_module;
  ad.name.assign(l->name);
  ad.description.assign(l->description);
  ad.monitorSourceName.assign(l->monitor_source_name);

  handle->push_back(ad);
}

void PAControl::pa_contextindex_cb(pa_context * c, unsigned index, void * h)
{
  (void)c;
  unsigned * handle = static_cast<unsigned*> (h);
  *handle = index;
}

void PAControl::pa_contextsuccess_cb(pa_context * c, int success, void * h)
{
  (void)c;
  (void)success;
  (void)h;
}
