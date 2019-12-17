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
    id: artistsPage
    objectName: "artistsPage"
    pageTitle: qsTr("Artists")
    pageFlickable: artistGridView
    searchable: true

    Component.onCompleted: {
        if (AllArtistsModel.isNew()) {
            AllArtistsModel.init(Sonos, "", false);
            AllArtistsModel.asyncLoad();
        }
    }

    onSearchClicked: filter.visible = true

    //Header
    MusicFilter {
        id: filter
        visible: false
    }

    MusicGridView {
        id: artistGridView
        anchors.topMargin: filter.visible ? filter.height : 0
        itemWidth: units.gu(12)
        heightOffset: units.gu(7)
        clip: true
        model: SortFilterModel {
            model: AllArtistsModel
            sort.property: "artist"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: Card {
            id: artistCard
            coverSources: makeCoverSource(undefined, model.artist, undefined)
            noCover: "qrc:/images/none.png"
            objectName: "artistsPageGridItem" + index
            primaryText: model.artist !== undefined && model.artist !== "" ? model.artist : qsTr("Unknown Artist")
            secondaryTextVisible: false
            isFavorite: (AllFavoritesModel.findFavorite(model.payload).length > 0)

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onCountChanged: {
                    artistCard.isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            onClicked: {
                pageStack.push("qrc:/sfos/pages/ArtistView.qml",
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "artistSearch": model.id,
                                       "artist": model.artist,
                                       "covers": [{art: artistCard.imageSource}],
                                       "pageTitle": qsTr("Artist")
                                   })
            }
            onPressAndHold: {
                if (isFavorite && removeFromFavorites(model.payload))
                    isFavorite = false
                else if (!isFavorite && addItemToFavorites(model, qsTr("Artist"), imageSource))
                    isFavorite = true
            }

            Component.onCompleted: {
                isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
            }
        }
    }
}
