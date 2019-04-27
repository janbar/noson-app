/*
 *      Copyright (C) 2019 Jean-Luc Barriere
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
#ifndef MEDIASCANNERPLUGIN_H
#define MEDIASCANNERPLUGIN_H

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlExtensionPlugin>

class MediaScannerPlugin : public QQmlExtensionPlugin
{

  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
  MediaScannerPlugin(QObject* parent = nullptr)
  : QQmlExtensionPlugin(parent) { }

  virtual void registerTypes(const char* uri) override;
  virtual void initializeEngine(QQmlEngine* engine, const char* uri) override;
  static QObject* createMediaScanner(QQmlEngine *engine, QJSEngine *scriptEngine);
};

#endif /* MEDIASCANNERPLUGIN_H */

