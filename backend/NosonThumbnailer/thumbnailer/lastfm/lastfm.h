/*
 *      Copyright (C) 2019 Jean-Luc Barriere
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

#ifndef LASTFM_H
#define LASTFM_H

#include "../abstractapi.h"

namespace thumbnailer
{

  class LastfmAPI final : public AbstractAPI
  {
  public:
    LastfmAPI() = default;
    virtual ~LastfmAPI() override = default;
    const char* name() override { return "LASTFM"; }
    int delayOnQuotaExceeded() override { return 3000; }
    int maxRetry() override { return 2; }
    bool configure(NetManager* nam, const QString& apiKey) override;
    AbstractArtistInfo* newArtistInfo(const QString& artist) override;
    AbstractAlbumInfo* newAlbumInfo(const QString& artist, const QString& album) override;

    static bool parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error);

  private:
    QString m_apikey;
  };

  class LFMArtistInfo final : public AbstractArtistInfo
  {
  public:
    LFMArtistInfo(const QString& apiKey, const QString& artist);
    virtual ~LFMArtistInfo() override = default;
    void queryInfo(NetRequest* prepared) override;
    AbstractAPI::Parse_Status parseInfo(const QByteArray& info, AbstractArtistInfo::metadata_t& meta) override;
    bool parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error) override;

  private:
    QString m_apiKey;
  };

  class LFMAlbumInfo final : public AbstractAlbumInfo
  {
  public:
    LFMAlbumInfo(const QString& apiKey, const QString& artist, const QString& album);
    virtual ~LFMAlbumInfo() override = default;
    void queryInfo(NetRequest* prepared) override;
    AbstractAPI::Parse_Status parseInfo(const QByteArray& info, AbstractAlbumInfo::metadata_t& meta) override;
    bool parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error) override;

  private:
    QString m_apiKey;
  };

}

#endif /* LASTFM_H */

