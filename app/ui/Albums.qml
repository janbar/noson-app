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

import QtQuick 2.9
import QtQuick.Controls 2.2
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

    MusicGridView {
        id: albumGridView
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)

        model: AllAlbumsModel
/*        model: SortFilterModel {
            id: albumsModelFilter
            model: AllAlbumsModel
            sort.property: "title"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(searchHeader.query), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }*/
        delegate: Card {
            id: albumCard
            coverSources: [
                {art: makeCoverSource(model.art, model.artist, model.title)},
                {art: makeCoverSource(undefined, model.artist, model.title)}
            ]
            objectName: "albumsPageGridItem" + index
            primaryText: model.title !== undefined && model.title !== "" ? model.title : qsTr("Unknown Album")
            secondaryText: model.artist !== undefined && model.artist !== "" ? model.artist : qsTr("Unknown Artist")

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onCountChanged: {
                    albumCard.isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            onClicked: {
                stackView.push("qrc:/ui/SongsView.qml",
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "songSearch": model.id,
                                       "album": model.title,
                                       "artist": model.artist,
                                       "covers": [{art: (albumCard.imageSource != "" ? albumCard.imageSource : model.art)}],
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
