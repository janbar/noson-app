/*
 *      Copyright (C) 2018 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
 *
 *  Noson-App is free software: you can redistribute it and/or modify
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

#include "plugin.h"

#include "albumartgenerator.h"
#include "artistartgenerator.h"

#include <QtQml>
#include <QtQml/QQmlContext>

#include <memory>

#define CACHE_SIZE 100000000L

namespace thumbnailer
{

  namespace qml
  {
    Proxy::Proxy(std::shared_ptr<thumbnailer::Thumbnailer>& thumbnailer, QObject *parent)
    : QObject(parent)
    , m_p(thumbnailer)
    {
    }
  } // namespace qml

} // namespace thumbnailer
