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

#include "thumbnailerimageresponse.h"

#include <QDebug>

namespace thumbnailer
{

  namespace qml
  {

    ThumbnailerImageResponse::ThumbnailerImageResponse(QSharedPointer<thumbnailer::Request> const& request)
    : request_(request)
    {
      Q_ASSERT(request);
      connect(request_.data(), &thumbnailer::Request::finished, this, &ThumbnailerImageResponse::requestFinished);
    }

    ThumbnailerImageResponse::ThumbnailerImageResponse(QString const& error_message)
    : error_message_(error_message)
    {
      // Queue the signal emission so there is time for the caller to connect.
      QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
    }

    ThumbnailerImageResponse::~ThumbnailerImageResponse()
    {
      cancel();
    }

    QQuickTextureFactory* ThumbnailerImageResponse::textureFactory() const
    {
      if (request_ && request_->isValid())
      {
        return QQuickTextureFactory::textureFactoryForImage(request_->image());
      }
      else
      {
        qWarning() << "ThumbnailerImageResponse::textureFactory(): method called without valid request.";
        return nullptr;
      }
    }

    QString ThumbnailerImageResponse::errorString() const
    {
      return error_message_;
    }

    void ThumbnailerImageResponse::cancel()
    {
      if (request_ && !request_->isFinished() && !request_->isCancelled())
      {
        request_->cancel();
      }
    }

    void ThumbnailerImageResponse::requestFinished()
    {
      if (!request_->isValid())
      {
        error_message_ = request_->errorMessage();
      }
      Q_EMIT finished();
    }

  } // namespace qml

} // namespace thumbnailer
