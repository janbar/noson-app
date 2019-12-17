/*
 * Copyright (C) 2016, 2017
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
import "../components"
import "../components/Delegates"
import "../components/Flickables"


MusicPage {
    id: albumsPage
    objectName: "albumsPage"
    pageTitle: qsTr("Albums")
    pageFlickable: albumGridView
    searchable: true

    Component.onCompleted: {
        if (AllAlbumsModel.isNew()) {
            AllAlbumsModel.init(Sonos, "", false);
            AllAlbumsModel.asyncLoad();
        }
    }

    onSearchClicked: filter.visible = true

    //Header
    MusicFilter {
        id: filter
        visible: false
    }

    MusicGridView {
        id: albumGridView
        anchors.topMargin: filter.visible ? filter.height : 0
        itemWidth: units.gu(15)
        heightOffset: units.gu(9)
        clip: true
        model: SortFilterModel {
            model: AllAlbumsModel
            sort.property: "title"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: Card {
            id: albumCard
            coverSources: makeCoverSource(model.art, model.artist, model.title)
            objectName: "albumsPageGridItem" + index
            primaryText: model.title !== undefined && model.title !== "" ? model.title : qsTr("Unknown Album")
            secondaryText: model.artist !== undefined && model.artist !== "" ? model.artist : qsTr("Unknown Artist")

            onImageError: model.art = "" // reset invalid url from model

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onCountChanged: {
                    albumCard.isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            onClicked: {
                pageStack.push("qrc:/sfos/pages/SongsView.qml",
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "songSearch": model.id,
                                       "album": model.title,
                                       "artist": model.artist,
                                       "covers": [{art: (albumCard.imageSource != "" ? albumCard.imageSource : coverSources)}],
                                       "isAlbum": true,
                                       "genre": "",
                                       "pageTitle": qsTr("Album"),
                                       "line1": model.artist !== undefined && model.artist !== "" ? model.artist : qsTr("Unknown Artist"),
                                       "line2": model.title !== undefined && model.title !== "" ? model.title : qsTr("Unknown Album")
                                   })
            }
            onPressAndHold: {
                if (isFavorite && removeFromFavorites(model.payload))
                    isFavorite = false
                else if (!isFavorite && addItemToFavorites(model, qsTr("Album"), imageSource))
                    isFavorite = true
            }

            Component.onCompleted: {
                isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
            }
        }
    }
}
