/*
 * Copyright 2015 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#ifndef THUMBNAILERIMAGERESPONSE_H
#define THUMBNAILERIMAGERESPONSE_H

#include <QQuickImageProvider>

#include "thumbnailer/thumbnailer.h"

namespace thumbnailer
{

  namespace qml
  {

    class ThumbnailerImageResponse : public QQuickImageResponse // LCOV_EXCL_LINE  // False negative from gcovr
    {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
      Q_OBJECT
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    public:
      Q_DISABLE_COPY(ThumbnailerImageResponse)

      ThumbnailerImageResponse(QSharedPointer<thumbnailer::Request> const& request);
      ThumbnailerImageResponse(QString const& error_message);
      ~ThumbnailerImageResponse();

      QQuickTextureFactory* textureFactory() const override;
      QString errorString() const override;
      void cancel() override;

      private
Q_SLOTS:
      void requestFinished();

    private:
      QSharedPointer<thumbnailer::Request> request_;
      QString error_message_;
    };

  } // namespace qml

} // namespace thumbnailer
#endif /* THUMBNAILERIMAGERESPONSE_H */

