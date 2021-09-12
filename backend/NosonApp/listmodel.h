/*
 *      Copyright (C) 2015-2019 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
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
 *  along with Noson.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NOSONAPPLISTMODEL_H
#define NOSONAPPLISTMODEL_H

#include "locked.h"
#include <noson/digitalitem.h>
#include <noson/sonoszone.h>
#include <noson/musicservices.h>
#include <noson/alarm.h>

#include <QObject>
#include <QRunnable>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#include <QRecursiveMutex>
#endif

Q_DECLARE_METATYPE(SONOS::DigitalItemPtr)
Q_DECLARE_METATYPE(SONOS::ZonePtr)
Q_DECLARE_METATYPE(SONOS::ZonePlayerPtr)
Q_DECLARE_METATYPE(SONOS::SMServicePtr)
Q_DECLARE_METATYPE(SONOS::AlarmPtr)

namespace nosonapp
{

template<class T> class ListModel;

template<class T>
class ContentProvider
{
public:
  virtual void beforeLoad() { }
  virtual void afterLoad() { }
  virtual void runContentLoader(ListModel<T>* model) { (void)model; }
  virtual void loadContent(ListModel<T>* model) { (void)model; }
  virtual void loadAllContent() { }
  virtual void runContentLoaderForContext(ListModel<T>* model, int id) { (void)model; (void)id; }
  virtual void loadContentForContext(ListModel<T>* model, int id) { (void)model; (void)id; }
  virtual const char* getHost() const { return nullptr; }
  virtual unsigned getPort() const { return 0; }
  virtual QString getBaseUrl() const { return QString(); }
  virtual void registerContent(ListModel<T>* model, const QString& root) { (void)model; (void)root; }
  virtual void unregisterContent(ListModel<T>* model) { (void)model; }
};

typedef enum {
  DataBlank     = 0,
  DataFailure   = 1,
  DataNotFound  = 2,
  DataLoaded    = 3,
  DataSynced    = 4
} DataStatus;

template<class T>
class ListModel
{
public:
  ListModel()
  : m_provider(nullptr)
  , m_lock(nullptr)
  , m_updateID(0)
  , m_root("")
  , m_pending(false)
  , m_dataState(DataBlank)
  , m_updateSignaled(false)
  {
  #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    m_lock = new QRecursiveMutex;
  #else
    m_lock = new QMutex(QMutex::Recursive);
  #endif
  }

  virtual ~ListModel()
  {
    {
      LockGuard g(m_lock);
      ContentProvider<T>* cp = (ContentProvider<T>*) m_provider;
      if (cp)
        cp->unregisterContent(this);
    }
    delete m_lock;
  }

  virtual void clearData() { }

  virtual bool loadData() { return false; }

  virtual void handleDataUpdate() { }

  virtual bool loadDataForContext(int id) { (void)id; return false; }

  virtual bool dataFailure() { return m_dataState == DataStatus::DataFailure; }

public:
  T* m_provider;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  QRecursiveMutex* m_lock;
#else
  QMutex* m_lock;
#endif
  unsigned m_updateID;
  QString m_root;
  bool m_pending;
  DataStatus m_dataState;

protected:
  virtual bool configure(T* provider, const QString& root, bool fill)
  {
    ContentProvider<T>* cp = (ContentProvider<T>*) provider;
    if (cp)
    {
      {
        LockGuard g(m_lock);
        if (m_provider)
          ((ContentProvider<T>*) m_provider)->unregisterContent(this);
        cp->registerContent(this, root);
        m_provider = provider;
        m_root = root;
        // Reset container status to allow async reload
        m_dataState = DataNotFound;
      }
      if (fill)
        return this->loadData();
    }
    return false; // not filled
  }

  virtual bool configure(T* provider, bool fill) { return configure(provider, QString(""), fill); }

  bool updateSignaled() { return m_updateSignaled.Load(); }
  void setUpdateSignaled(bool val) { m_updateSignaled.Store(val); }

private:
  Locked<bool> m_updateSignaled;
};

template<class T>
struct RegisteredContent
{
  RegisteredContent(ListModel<T>* model, const QString& root)
  : model(model)
  , root(root) { }
  ListModel<T>* model;
  QString root;
};

template<class T>
class ContentLoader : public QRunnable
{
public:
  ContentLoader(ContentProvider<T>& provider, ListModel<T>* payload)
  : m_provider(provider)
  , m_payload(payload) { }

  ContentLoader(ContentProvider<T>& provider)
  : m_provider(provider)
  , m_payload(nullptr) { }

  virtual void run() override
  {
    m_provider.beforeLoad();
    if (m_payload)
      m_provider.loadContent(m_payload);
    else
      m_provider.loadAllContent();
    m_provider.afterLoad();
  }
private:
  ContentProvider<T>& m_provider;
  ListModel<T>* m_payload;
};

template<class T>
class ContentForContextLoader : public QRunnable
{
public:
  ContentForContextLoader(ContentProvider<T>& provider, ListModel<T>* payload, int id)
  : m_provider(provider)
  , m_payload(payload)
  , m_id(id) { }

  virtual void run() override
  {
    m_provider.beforeLoad();
    if (m_payload)
      m_provider.loadContentForContext(m_payload, m_id);
    m_provider.afterLoad();
  }
private:
  ContentProvider<T>& m_provider;
  ListModel<T>* m_payload;
  int m_id;
};

}

#endif // NOSONAPPLISTMODEL_H

