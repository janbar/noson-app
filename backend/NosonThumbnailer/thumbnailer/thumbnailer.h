/*
 * Copyright (C) 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 *              Jean-Luc Barriere <jlbarriere68@gmail.com>
 */

#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <QImage>
#include <QObject>
#include <QSharedPointer>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>

namespace thumbnailer
{
  class ThumbnailerImpl;
  class RequestImpl;
  class NetManager;

  /**
  \brief Holds a thumbnailer request.

  This class stores the state of an in-progress or completed thumbnail request.
   */

  class Q_DECL_EXPORT Request : public QObject
  {
    Q_OBJECT
  public:
    Q_DISABLE_COPY(Request)

    /**
    \brief Destroys a request.

    If the request is still in progress, it is implicitly cancelled, and
    the finished() signal is _not_ emitted.

    \warning You _must_ destroy all request instances created by a Thumbnailer _before_
    destroying the Thumbnailer instance.
     */
    ~Request();

    /**
    \brief Returns the details.
    \return The details of the request.
     */

    QString details() const;

    /**
    \brief Returns whether the request has completed.

    \return `false` if the request is still in progress. Otherwise, the return
    value is `true` (whether the request completed successfully or not).
     */
    bool isFinished() const;

    /**
    \brief Returns the thumbnail.
    \return A valid `QImage` if the request was successful and an empty `QImage`, otherwise.
     */
    QImage image() const;

    /**
    \brief Returns the error message for a failed request.
    \return The error message in case of a failure and an empty `QString`, otherwise.
     */
    QString errorMessage() const;

    /**
    \brief Returns whether the request completed successfully.
    \return `true` if the request completed successfully. Otherwise, if the request is still
    in progress, has failed, or was cancelled, the return value is `false`.
     */
    bool isValid() const;

    /**
    \brief Blocks the calling thread until the request completes.

    It is safe to call waitForFinished() on the same request more than once.
    If called on an already-completed (or cancelled) request, waitForFinished() returns immediately.

    \warning Calling this function from the main (GUI) thread might cause your user interface to freeze.

    \warning Calling waitForFinished() causes the request to be scheduled out of order. This means
    that, if you send requests for thumbnails A, B, and C (in that order) and then call waitForFinished()
    on C, you _cannot_ assume that A and B have also finished once waitForFinished() returns.
     */
    void waitForFinished();

    /**
    \brief Cancel the thumbnail request.

    Cancels the request if it has not completed yet and emits the finished() signal.
    Calling cancel() more than once or on a request that has already completed does nothing.
     */
    void cancel();

    /**
    \brief Returns whether the request was cancelled.
    \return `true` if the request was cancelled and `false`, otherwise.
    \note Depending on the time at which cancel() is called,
          the request may complete successfully despite having been cancelled.
     */
    bool isCancelled() const;

    signals:
    /**
    \brief This signal is emitted when the request completes.
     */
    void finished();

  public slots:
    void start();

  private:
    QScopedPointer<RequestImpl> p_;
    explicit Request(RequestImpl* impl) Q_DECL_HIDDEN;

    friend class ThumbnailerImpl;
  };

  /**
  \brief Class to obtain thumbnail images for music media types.

  Class Thumbnailer provides thumbnail images for album covers and artist images for many musicians
  and bands. Artwork is downloaded from a remote server that maintains a large database of
  albums and musicians.

  The requested size for a thumbnail specifies a bounding box (in pixels) of type `QSize` to which
  the thumbnail will be scaled.
  (The aspect ratio of the original image is preserved.)

  - Passing `QSize(0,0)` requests a thumbnail with largest size.
  - Sizes with one or both dimensions &lt;&nbsp;0 return an error.

  Original images are never scaled up, so the returned thumbnail may be smaller than its requested size.

  All methods are asynchronous and are guaranteed not to block.

  The return value is a shared pointer to a \link thumbnailer::Request Request\endlink instance that
  provides access to the scaled thumbnail (or an error message).
   */

  class Q_DECL_EXPORT Thumbnailer final
  {
  public:
    Q_DISABLE_COPY(Thumbnailer)

    /**
    \brief Constructs a thumbnailer instance.

    \warning Instantiation and finalization of Thumbnailer instances are expensive
    operations. Do not needlessly destroy a Thumbnailer only to re-create it again later.

    \param nam The network manager to send requests.
    \param apiKey A valid API key for the service.

    \warning Instantiation and finalization of Thumbnailer instances are expensive
    operations. Do not needlessly destroy a Thumbnailer only to re-create it again later.
     */
    explicit Thumbnailer(const QString& offlineStoragePath,
            qint64 maxCacheSize);

    /**
    \brief Destroys a thumbnailer instance.

    \warning You _must_ keep the Thumbnailer instance alive for as long as
    Request instances created by that Thumbnailer instance exist.
     */
    ~Thumbnailer();

    /**
    \brief Retrieves a thumbnail for an album cover from the remote image server.
    \param artist The name of the artist.
    \param album The name of the album.
    \param requestedSize The bounding box for the thumbnail.
    \return A `QSharedPointer` to a thumbnailer::Request holding the request state.
     */
    QSharedPointer<Request> getAlbumArt(QString const& artist, QString const& album, QSize const& requestedSize);

    /**
    \brief Retrieves a thumbnail for an artist from the remote image server.
    \param artist The name of the artist.
    \param requestedSize The bounding box for the thumbnail.
    \return A `QSharedPointer` to a thumbnailer::Request holding the request state.
     */
    QSharedPointer<Request> getArtistArt(QString const& artist, QSize const& requestedSize);

    bool isValid();

    void configure(const QString& apiName, const QString& apiKey);

    QString apiName();

    void setTrace(bool trace_client);

    void clearCache();

    void reset();

  private:
    QScopedPointer<ThumbnailerImpl> p_;
  };

}
#endif /* THUMBNAILER_H */

