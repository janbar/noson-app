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
 *              Michi Henning <michi.henning@canonical.com>
 *              Jean-Luc Barriere <jlbarriere68@gmail.com>
 */

#include "thumbnailer.h"
#include "ratelimiter.h"
#include "lfm-artistinfo.h"
#include "lfm-albuminfo.h"
#include "diskcachemanager.h"
#include "netmanager.h"

#include <QNetworkReply>
#include <QSharedPointer>
#include <QDebug>

#include <memory>


#define MAX_BACKLOG 4   // Maximum number of pending requests before the thumbnailer starts queuing them.
#define MAX_NETWORK_ERROR 2

namespace thumbnailer
{

  class RequestImpl : public QObject
  {
    Q_OBJECT
  public:
    RequestImpl(QString const& details,
            QSize const& requested_size,
            ThumbnailerImpl& thumbnailer,
            Job* job,
            bool trace_client);

    ~RequestImpl();

    void start();
    QString details() const { return details_; }
    bool isFinished() const { return finished_; }
    QImage image() const { return image_; }
    QString errorMessage() const { return error_message_; }
    bool isValid() const { return is_valid_; }
    void waitForFinished();
    void setRequest(Request* request) { public_request_ = request; }
    void cancel();
    bool isCancelled() const { return cancelled_; }

  private slots:
    void callFinished();

  private:
    void finishWithError(QString const& errorMessage);

    QString details_;
    QSize requested_size_;
    ThumbnailerImpl& thumbnailer_;
    std::unique_ptr<Job> job_;
    std::function<void() > send_request_;

    RateLimiter::CancelFunc cancel_func_;
    QString error_message_;
    bool finished_;
    bool is_valid_;
    bool cancelled_; // true if cancel() was called by client
    bool cancelled_while_waiting_; // true if cancel() succeeded because request was not sent yet
    bool trace_client_;
    QImage image_;
    Request* public_request_;
  };

  class ThumbnailerImpl : public QObject
  {
    Q_OBJECT
  public:
    Q_DISABLE_COPY(ThumbnailerImpl)
    explicit ThumbnailerImpl(const QString& offlineStoragePath,
            qint64 maxCacheSize,
            const QString& apiKey);
    ~ThumbnailerImpl();

    bool isValid() const;
    void setApiKey(const QString& apiKey);
    void setTrace(bool trace_client);
    void clearCache();
    void reset();
    
    QSharedPointer<Request> getAlbumArt(QString const& artist, QString const& album, QSize const& requestedSize);
    QSharedPointer<Request> getArtistArt(QString const& artist, QSize const& requestedSize);
    QSharedPointer<Request> getThumbnail(QString const& filename, QSize const& requestedSize);

    RateLimiter& limiter();
    Q_INVOKABLE void pump_limiter();

  public slots:
    void onNetworkError(); // will provide data only from the cache
    void onFatalError(); // will reject any future request

  private:
    QSharedPointer<Request> createRequest(QString const& details,
            QSize const& requested_size,
            Job* job);

    bool trace_client_;
    RateLimiter* limiter_;
    DiskCacheManager* cache_;
    NetManager* nam_;
    QString apiKey_;
    bool valid_;

    bool netFailed_;
    std::atomic<int> nwerr_;
    std::atomic<int> fatal_;
  };


  
  RequestImpl::RequestImpl(QString const& details,
          QSize const& requested_size,
          ThumbnailerImpl& thumbnailer,
          Job* job,
          bool trace_client)
  : details_(details)
  , requested_size_(requested_size)
  , thumbnailer_(thumbnailer)
  , job_(job)
  , finished_(false)
  , is_valid_(false)
  , cancelled_(false)
  , cancelled_while_waiting_(false)
  , trace_client_(trace_client)
  , public_request_(nullptr)
  {
    if (!job_)
    {
      finished_ = true;
      return;
    }
    if (!requested_size.isValid())
    {
      error_message_ = details_ + ": " + "invalid QSize";
      qCritical().noquote() << error_message_;
      finished_ = true;
      return;
    }
  }

  RequestImpl::~RequestImpl()
  {
    // If cancel_func_() returns false and we have a pending reply,
    // the request was sent but the reply has not yet trickled in.
    // We have to pump the limiter in that case because we'll never
    // receive the callFinished callback.
    if (job_  && cancel_func_ && !cancel_func_())
    {
      // Delay pumping until we drop back to the event loop. Otherwse,
      // if the caller destroys a whole bunch of requests at once, we'd
      // schedule the next request in the queue before the caller gets
      // a chance to destroy the next request.
      QMetaObject::invokeMethod(&thumbnailer_, "pump_limiter", Qt::QueuedConnection);
      disconnect();
    }
  }

  void RequestImpl::callFinished()
  {
    Q_ASSERT(!finished_);

    // If this isn't a fake call from cancel(), pump the limiter.
    if (!cancelled_ || !cancelled_while_waiting_)
    {
      // We depend on calls to pump the limiter exactly once for each request that was sent.
      // Whenever a (real) DBus call finishes, we inform the limiter, so it can kick off
      // the next pending job.
      thumbnailer_.limiter().done();
    }

    if (cancelled_)
    {
      finishWithError("Request cancelled");
      Q_ASSERT(!job_);
      return;
    }

    Q_ASSERT(job_);
    Q_ASSERT(!finished_);

    if (job_->error() != ReplySuccess)
    {
      switch(job_->error())
      {
        case ReplyNetworkError:
          thumbnailer_.onNetworkError();
          break;
        case ReplyFatalError:
          thumbnailer_.onFatalError();
          break;
        default:
          break;
      }
      finishWithError("Thumbnailer: " + job_->errorString());
      return;
    }

    try
    {
      image_ = QImage::fromData(job_->image());
      finished_ = true;
      is_valid_ = true;
      error_message_ = QLatin1String("");
      Q_ASSERT(public_request_);
      emit public_request_->finished();
      if (trace_client_)
      {
        qDebug().noquote() << "Thumbnailer: completed:" << details_;
      }
      job_.reset();
    }
    // LCOV_EXCL_START
    catch (const std::exception& e)
    {
      finishWithError("Thumbnailer: RequestImpl::callFinished(): thumbnailer failed: " +
              QString::fromStdString(e.what()));
    }
    catch (...)
    {
      finishWithError(QStringLiteral("Thumbnailer: RequestImpl::callFinished(): unknown exception"));
    }
    // LCOV_EXCL_STOP
  }

  void RequestImpl::finishWithError(QString const& errorMessage)
  {
    error_message_ = errorMessage;
    finished_ = true;
    is_valid_ = false;
    image_ = QImage();
    if (trace_client_)
    {
      if (!cancelled_)
        qDebug().noquote() << error_message_; // Cancellation is an expected outcome
      else
        qDebug().noquote() << "Thumbnailer: cancelled:" << details_;
    }
    job_.reset();
    Q_ASSERT(public_request_);
    emit public_request_->finished();
  }

  void RequestImpl::start()
  {
    // Return immediately if the request was canceled before event is running.
    if (cancelled_)
    {
      return;
    }
    // The limiter does not call send_request_ until the request can be sent
    // without exceeding max_backlog().
    send_request_ = [this] {
      connect(job_.get(), SIGNAL(finished()), this, SLOT(callFinished()));
      job_->start();
    };
    cancel_func_ = thumbnailer_.limiter().schedule(send_request_);
  }

  void RequestImpl::waitForFinished()
  {
    if (finished_ || cancelled_)
    {
      return;
    }

    // If we are called before the request made it out of the limiter queue,
    // we have not sent the request yet and, therefore, don't have a pending
    // reply. In that case we send the request right here after removing it
    // from the limiter queue. This guarantees that we always have a reply
    // to wait on.
    if (cancel_func_())
    {
      Q_ASSERT(!job_);
      thumbnailer_.limiter().schedule_now(send_request_);
    }
  }

  void RequestImpl::cancel()
  {
    if (trace_client_)
    {
      qDebug().noquote() << "Thumbnailer: cancelling:" << details_;
    }

    if (finished_ || cancelled_)
    {
      if (trace_client_)
      {
        qDebug().noquote() << "Thumbnailer: already finished or cancelled:" << details_;
      }
      return; // Too late, do nothing.
    }

    cancelled_ = true;
    cancelled_while_waiting_ = cancel_func_ && cancel_func_();
    if (cancelled_while_waiting_)
    {
      // We fake the call completion, in order to pump the limiter only from within
      // the dbus completion callback. We cannot call thumbnailer_.limiter().done() here
      // because that would schedule the next request in the queue.
      QMetaObject::invokeMethod(this, "callFinished", Qt::QueuedConnection);
    }
  }

  ThumbnailerImpl::ThumbnailerImpl(const QString& offlineStoragePath,
            qint64 maxCacheSize,
            const QString& apiKey)
  : QObject(nullptr)
  , trace_client_(false)
  , limiter_(nullptr)
  , cache_(nullptr)
  , nam_(nullptr)
  , apiKey_(apiKey)
  , valid_(false)
  , netFailed_(false)
  , nwerr_(0)
  , fatal_(0)
  {
    qInfo().noquote() << "installing thumbnails cache in folder \"" + offlineStoragePath + "\"";
    limiter_ = new RateLimiter(MAX_BACKLOG);
    cache_ = new DiskCacheManager(offlineStoragePath, maxCacheSize);
    nam_ = new NetManager();
    qInfo().noquote() << "thumbnailer Last.fm is initialized";
    valid_ = !apiKey_.isEmpty();

    // initialize the random generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));
  }

  ThumbnailerImpl::~ThumbnailerImpl()
  {
    delete nam_;
    delete cache_;
    delete limiter_;
  }

  bool ThumbnailerImpl::isValid() const
  {
    return valid_;
  }

  void ThumbnailerImpl::setApiKey(const QString &apiKey)
  {
    qInfo().noquote() << "thumbnailer: configure key [" + apiKey + "]";
    apiKey_ = apiKey;
    fatal_.store(0); // reset fatal event count
    valid_ = !apiKey_.isEmpty();
  }

  void ThumbnailerImpl::setTrace(bool trace_client)
  {
    qInfo().noquote() << "thumbnailer: enable trace client";
    trace_client_ = trace_client;
  }

  void ThumbnailerImpl::clearCache()
  {
    cache_->clear();
  }

  void ThumbnailerImpl::reset()
  {
    nwerr_.store(0);
    netFailed_ = false;
    fatal_.store(0);
    valid_ = !apiKey_.isEmpty();
  }

  QSharedPointer<Request> ThumbnailerImpl::getAlbumArt(QString const& artist, QString const& album, QSize const& requestedSize)
  {
    QString details;
    QTextStream s(&details, QIODevice::WriteOnly);
    s << "getAlbumArt: (" << requestedSize.width() << "," << requestedSize.height()
            << ") \"" << artist << "\", \"" << album << "\"";
    Job* job = new Job(new AlbumInfo(cache_, nam_, apiKey_, artist, album, requestedSize, netFailed_));
    return createRequest(details, requestedSize, job);
  }

  QSharedPointer<Request> ThumbnailerImpl::getArtistArt(QString const& artist, QSize const& requestedSize)
  {
    QString details;
    QTextStream s(&details, QIODevice::WriteOnly);
    s << "getArtistArt: (" << requestedSize.width() << "," << requestedSize.height()
            << ") \"" << artist << "\"";
    Job* job = new Job(new ArtistInfo(cache_, nam_, apiKey_, artist, requestedSize, netFailed_));
    return createRequest(details, requestedSize, job);
  }

  QSharedPointer<Request> ThumbnailerImpl::createRequest(QString const& details,
          QSize const& requested_size,
          Job* job)
  {
    if (trace_client_)
    {
      qDebug().noquote() << "Thumbnailer:" << details;
    }
    auto request_impl = new RequestImpl(details, requested_size, *this, job, trace_client_);
    auto request = QSharedPointer<Request>(new Request(request_impl));
    if (request->isFinished())
      QMetaObject::invokeMethod(request.data(), "finished", Qt::QueuedConnection);
    else
      QMetaObject::invokeMethod(request.data(), "start", Qt::QueuedConnection);
    return request;
  }

  RateLimiter& ThumbnailerImpl::limiter()
  {
    return *limiter_;
  }

  void ThumbnailerImpl::pump_limiter()
  {
    return limiter_->done();
  }

  void ThumbnailerImpl::onNetworkError()
  {
    if (nwerr_.fetch_add(1) > MAX_NETWORK_ERROR && !netFailed_)
    {
      qWarning().noquote() << "thumbnailer: remote call suspended due to network error";
      netFailed_ = true;
    }
  }

  void ThumbnailerImpl::onFatalError()
  {
    if (fatal_.fetch_add(1) >= 0 && valid_)
    {
      qWarning().noquote() << "thumbnailer: service suspended due to fatal error";
      valid_ = false;
    }
  }

  Request::Request(RequestImpl* impl)
  : p_(impl)
  {
    impl->setRequest(this);
  }

  Request::~Request() = default;

  void Request::start()
  {
    p_->start();
  }

  QString Request::details() const
  {
    return p_->details();
  }

  bool Request::isFinished() const
  {
    return p_->isFinished();
  }

  QImage Request::image() const
  {
    return p_->image();
  }

  QString Request::errorMessage() const
  {
    return p_->errorMessage();
  }

  bool Request::isValid() const
  {
    return p_->isValid();
  }

  void Request::waitForFinished()
  {
    p_->waitForFinished();
  }

  void Request::cancel()
  {
    p_->cancel();
  }

  bool Request::isCancelled() const
  {
    return p_->isCancelled();
  }

  Thumbnailer::Thumbnailer(const QString& offlineStoragePath,
          qint64 maxCacheSize,
          const QString& apiKey)
  : p_(new ThumbnailerImpl(offlineStoragePath, maxCacheSize, apiKey))
  {
  }

  Thumbnailer::~Thumbnailer() = default;
  
  QSharedPointer<Request> Thumbnailer::getAlbumArt(QString const& artist, QString const& album, QSize const& requestedSize)
  {
    return p_->getAlbumArt(artist, album, requestedSize);
  }

  QSharedPointer<Request> Thumbnailer::getArtistArt(QString const& artist, QSize const& requestedSize)
  {
    return p_->getArtistArt(artist, requestedSize);
  }

  bool Thumbnailer::isValid()
  {
    return p_->isValid();
  }

  void Thumbnailer::setApiKey(const QString &apiKey)
  {
    p_->setApiKey(apiKey);
  }

  void Thumbnailer::setTrace(bool trace_client)
  {
    p_->setTrace(trace_client);
  }

  void Thumbnailer::clearCache()
  {
    p_->clearCache();
  }

  void Thumbnailer::reset()
  {
    p_->reset();
  }

}

#include "thumbnailer.moc"
