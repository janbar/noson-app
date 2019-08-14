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

#ifndef DEEZER_H
#define DEEZER_H

#include "../abstractapi.h"

namespace thumbnailer
{

  class DeezerAPI final : public AbstractAPI
  {
  public:
    DeezerAPI() = default;
    virtual ~DeezerAPI() override = default;
    const char* name() override { return "DEEZER"; }
    int delayOnQuotaExceeded() override { return 2000; }
    int maxRetry() override { return 3; }
    bool configure(NetManager* nam, const QString& apiKey) override;
    AbstractArtistInfo* newArtistInfo(const QString& artist) override;
    AbstractAlbumInfo* newAlbumInfo(const QString& artist, const QString& album) override;

    static bool parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error);
  };

  class DEEZERArtistInfo final : public AbstractArtistInfo
  {
  public:
    DEEZERArtistInfo(const QString& artist);
    virtual ~DEEZERArtistInfo() override = default;
    void queryInfo(NetRequest* prepared) override;
    AbstractAPI::Parse_Status parseInfo(const QByteArray& info, AbstractArtistInfo::metadata_t& meta) override;
    bool parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error) override;
  };

  class DEEZERAlbumInfo final : public AbstractAlbumInfo
  {
  public:
    DEEZERAlbumInfo(const QString& artist, const QString& album);
    virtual ~DEEZERAlbumInfo() override = default;
    void queryInfo(NetRequest* prepared) override;
    AbstractAPI::Parse_Status parseInfo(const QByteArray& info, AbstractAlbumInfo::metadata_t& meta) override;
    bool parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error) override;
  };

}

#endif /* DEEZER_H */

