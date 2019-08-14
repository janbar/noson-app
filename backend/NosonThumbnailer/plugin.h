/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
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

#ifndef THUMBNAILERPLUGIN_H
#define THUMBNAILERPLUGIN_H

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlExtensionPlugin>

#include "thumbnailer/thumbnailer.h"

#include <memory>

class ThumbnailerPlugin : public QQmlExtensionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
  ThumbnailerPlugin(QObject* parent = nullptr);

  virtual void registerTypes(const char* uri) override;
  virtual void initializeEngine(QQmlEngine* engine, const char* uri) override;

  static QObject* proxy(QQmlEngine *engine, QJSEngine *scriptEngine);

private:
  static std::shared_ptr<::thumbnailer::Thumbnailer> g_thumbnailer;
};

namespace thumbnailer
{

class Proxy : public QObject
{
  Q_OBJECT

public:
  Proxy(std::shared_ptr<Thumbnailer>& thumbnailer, QObject* parent = nullptr);
  virtual ~Proxy() { }

  Q_INVOKABLE bool configure(const QString& apiName, const QString& apiKey);

  Q_INVOKABLE void setTrace(bool trace) { m_p->setTrace(trace); }

  Q_INVOKABLE void clearCache() { m_p->clearCache(); }

  Q_INVOKABLE void reset() { m_p->reset(); }

private:
  std::shared_ptr<Thumbnailer> m_p;
};

} // namespace thumbnailer

#endif /* THUMBNAILERPLUGIN_H */

