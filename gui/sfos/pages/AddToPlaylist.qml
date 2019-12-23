/*
 * Copyright (C) 2013-2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Adam Pigg <adam@piggz.co.uk>
 *      Andrew Hayzen <ahayzen@gmail.com>
 *      Daniel Holm <d.holmen@gmail.com>
 *      Victor Thompson <victor.thompson@gmail.com>
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
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/Dialog"


MusicPage {
    id: addToPlaylistPage
    objectName: "addToPlaylistPage"

    //: this appears in the header with limited space (around 20 characters)
    pageTitle: qsTr("Select playlist")
    pageFlickable: addtoPlaylistView
    searchable: AllPlaylistsModel.count > 1

    state: "addToPlaylist"

    property var chosenElements: []

    onVisibleChanged: {
        // Load the playlistmodel if it hasn't loaded or is empty
        if (visible && (AllPlaylistsModel.count === 0)) {
            customdebug("AllPlaylistsModel is empty !!!")
            AllPlaylistsModel.asyncLoad()
        }
    }

    onSearchClicked: filter.visible = true

    //Header
    MusicFilter {
        id: filter
        visible: false
        onVisibleChanged: showToolbar = !visible
    }

    MusicGridView {
        id: addtoPlaylistView
        itemWidth: units.gu(12)
        heightOffset: units.gu(7)
        model: SortFilterModel {
            model: AllPlaylistsModel
            sort.property: "title"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }
        clip: true
        delegate: Card {
            id: playlist
            coverSources: [{art: model.art}]
            primaryText: model.title
            secondaryTextVisible: false

            onClicked: {
                songPlaylistLoader.model = model;
                addToPlaylistPage.addItemList(model.id, chosenElements, songPlaylistLoader.item.containerUpdateID());
                songPlaylistLoader.sourceComponent = null;
                pageStack.pop();
            }
        }
    }

    onAddClicked: dialogNewPlaylist.open()

    // Load selected container playlist
    Loader {
        id: songPlaylistLoader
        sourceComponent: Component {
            TracksModel {
            }
        }
        property var model: null
        asynchronous: false
        onModelChanged: {
            if (model)
                item.init(Sonos, model.id, true);
        }
    }

    // Overlay to show when no playlists are on the device
    Loader {
        anchors.fill: parent
        active: AllPlaylistsModel.count === 0 && !infoLoadedIndex
        asynchronous: true
        source: "../components/PlaylistsEmptyState.qml"
        visible: active
    }

    // Add chosen elements
    function addItemList(playlistId, modelItems, containerUpdateID) {
        if (modelItems.length > 0) {
            var eqcount = 0;
            for (var i = 0; i < modelItems.length; i++) {
                containerUpdateID = player.addItemToSavedQueue(playlistId, modelItems[i], containerUpdateID);
                if (!containerUpdateID || ++eqcount >= queueBatchSize) // limit batch size
                    break;
            }
            if (eqcount > 0) {
                popInfo.open(qsTr("song added"));
                return true;
            }
            popInfo.open(qsTr("Action can't be performed"));
            return false;
        }
    }
}
