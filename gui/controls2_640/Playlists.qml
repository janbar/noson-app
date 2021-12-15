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
import QtQuick.Layouts 1.3
import NosonApp 1.0
import "components"
import "components/Delegates"
import "components/Flickables"
import "components/Dialog"

MusicPage {
    id: playlistsPage
    objectName: "playlistsPage"
    pageTitle: qsTr("Playlists")
    pageFlickable: playlistsGrid
    searchable: true
    addVisible: true

    property bool changed: false
    property bool childrenChanged: false

    onSearchClicked: filter.visible = true

    header: MusicFilter {
        id: filter
        visible: false
    }

    SortFilterModel {
        id: playlistsModelFilter
        model: AllPlaylistsModel
        sort.property: "title"
        sort.order: Qt.AscendingOrder
        sortCaseSensitivity: Qt.CaseInsensitive
        filter.property: "normalized"
        filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
        filterCaseSensitivity: Qt.CaseInsensitive
    }

    MusicGridView {
        id: playlistsGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(7)

        model: playlistsModelFilter

        delegate: Card {
            id: playlistCard
            height: playlistsGrid.cellHeight
            width: playlistsGrid.cellWidth
            primaryText: model.title
            secondaryTextVisible: false

            isFavorite: (AllFavoritesModel.findFavorite(model.payload).length > 0)

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                function onLoaded(succeeded) {
                    isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            coverSources: allCovers()

            function allCovers() {
                var cs = [];
                var arts = model.arts;
                for (var i = 0; i < arts.length; ++i) cs.push({art: arts[i]});
                return cs;
            }

            coverFlow: 4

            onClicked: {
                stackView.push("qrc:/controls2/TrackStackView.qml",
                               {
                                   "containerItem": makeContainerItem(model),
                                   "containerTitle": model.title,
                                   "containerId": model.id,
                                   "covers": coverSources,
                                   "pageTitle": qsTr("Playlist")
                               })
            }
            onPressAndHold: {
                if (isFavorite && removeFromFavorites(model.payload))
                    isFavorite = false
                else if (!isFavorite && addItemToFavorites(model, qsTr("Playlist"), imageSource))
                    isFavorite = true
            }
        }
    }

    // Overlay to show when no playlists are on the device
    Loader {
        anchors.fill: parent
        active: AllPlaylistsModel.dataState === Sonos.DataSynced && AllPlaylistsModel.count === 0
        asynchronous: true
        source: "qrc:/controls2/components/PlaylistsEmptyState.qml"
        visible: active
    }

    DialogNewPlaylist {
        id: dialogNewPlaylist
    }

    onAddClicked: dialogNewPlaylist.open()

    // Overlay to show when load failed
    Loader {
        anchors.fill: parent
        active: AllPlaylistsModel.failure
        asynchronous: true
        sourceComponent: Component {
            DataFailureState {
                onReloadClicked: AllPlaylistsModel.asyncLoad();
            }
        }
        visible: active
    }
}
