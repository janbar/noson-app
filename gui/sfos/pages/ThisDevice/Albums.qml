/*
 * Copyright (C) 2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.2
import Sailfish.Silica 1.0
import QtQuick.Layouts 1.1
import NosonApp 1.0
import NosonMediaScanner 1.0
import "../../components"
import "../../components/Delegates"
import "../../components/Flickables"


MusicPage {
    id: albumsPage
    objectName: "albumsPage"
    pageFlickable: albumGridView
    pageMenuEnabled: false
    searchable: true

    AlbumList {
        id: albums
        Component.onCompleted: init()
    }

    function makeFileCoverSource(hasArt, filePath, artist, album) {
        var art = "";
        if (hasArt)
            art = player.makeFilePictureLocalURL(filePath);
        return makeCoverSource(art, artist, album);
    }

    onSearchClicked: filter.visible = true

    //Header
    MusicFilter {
        id: filter
        visible: false
        onVisibleChanged: showToolbar = !visible
    }

    MusicGridView {
        id: albumGridView
        anchors.topMargin: filter.visible ? filter.height : 0
        itemWidth: units.gu(15)
        heightOffset: units.gu(9)
        clip: true
        model: SortFilterModel {
            model: albums
            sort.property: "album"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: Card {
            id: albumCard
            coverSources: makeFileCoverSource(model.hasArt, model.filePath, model.artist, model.album)
            objectName: "albumsPageGridItem" + index
            primaryText: (model.album !== "<Undefined>" ? model.album : tr_undefined)
            secondaryText: (model.artist !== "<Undefined>" ? model.artist : tr_undefined)

            onImageError: model.art = "" // reset invalid url from model

            onClicked: {
                pageStack.push("qrc:/sfos/pages/ThisDevice/SongsView.qml",
                                   {
                                       "album": model.album,
                                       "artist": model.artist,
                                       "covers": [{art: (albumCard.imageSource != "" ? albumCard.imageSource : coverSources)}],
                                       "isAlbum": true,
                                       "genre": "",
                                       "pageTitle": pageTitle,
                                       "line1": (model.artist !== "<Undefined>" ? model.artist : tr_undefined),
                                       "line2": (model.album !== "<Undefined>" ? model.album : tr_undefined)
                                   })
            }
        }
    }
}
