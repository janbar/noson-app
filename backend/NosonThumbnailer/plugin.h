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

#ifndef THUMBNAILERPLUGIN_H
#define THUMBNAILERPLUGIN_H

#include "thumbnailer/thumbnailer.h"

#include <memory>

namespace thumbnailer
{

  namespace qml
  {
    class Proxy : public QObject
    {
      Q_OBJECT

    public:
      Proxy(std::shared_ptr<thumbnailer::Thumbnailer>& thumbnailer, QObject* parent = 0);
      ~Proxy() = default;

      Q_INVOKABLE bool setApiKey(const QString& apiKey)
      {
          m_p->setApiKey(apiKey);
          return m_p->isValid();
      }

      Q_INVOKABLE void setTrace(bool trace) { m_p->setTrace(trace); }

      Q_INVOKABLE void clearCache() { m_p->clearCache(); }

      Q_INVOKABLE void reset() { m_p->reset(); }

    private:
      std::shared_ptr<thumbnailer::Thumbnailer> m_p;
    };

  } // namespace qml

} // namespace thumbnailer
#endif /* THUMBNAILERPLUGIN_H */

