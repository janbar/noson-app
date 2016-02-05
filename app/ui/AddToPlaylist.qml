/*
 * Copyright (C) 2013, 2014, 2015, 2016
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
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

import QtMultimedia 5.0
import QtQuick 2.4
import Ubuntu.Components 1.2
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"


MusicPage {
    id: addToPlaylistPage
    objectName: "addToPlaylistPage"
    // TRANSLATORS: this appears in the header with limited space (around 20 characters)
    title: i18n.tr("Select playlist")
    searchable: true
    searchResultsCount: addToPlaylistModelFilter.count
    showToolbar: false
    state: "default"
    states: [
        PlaylistsHeadState {
            newPlaylistEnabled: true
            searchEnabled: AllPlaylistsModel.count > 0
            thisPage: addToPlaylistPage
        },
        SearchHeadState {
            id: searchHeader
            thisPage: addToPlaylistPage
        }
    ]

    // FIXME: workaround for pad.lv/1531016 (gridview juddery)
    anchors {
        fill: undefined
    }
    height: mainView.height
    width: mainView.width

    property var chosenElements: []

    onVisibleChanged: {
        // Load the playlistmodel if it hasn't loaded or is empty
        if (visible && (AllPlaylistsModel.count === 0)) {
            customdebug("#### AllPlaylistsModel is empty !!!")
        }
    }

    MusicGridView {
        id: addtoPlaylistView
        itemWidth: units.gu(12)
        heightOffset: units.gu(9.5)
        model: SortFilterModel {
            // Sorting disabled as it is incorrect on first run (due to workers?)
            // and SQL sorts the data correctly
            id: addToPlaylistModelFilter
            model: AllPlaylistsModel
            filter.property: "title"
            filter.pattern: new RegExp(searchHeader.query, "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }
        objectName: "addToPlaylistGridView"
        delegate: Card {
            id: playlist
            coverSources: [{art: model.art}]
            objectName: "addToPlaylistCardItem" + index

            primaryText: model.title
            secondaryTextVisible: false

            onClicked: {
                songPlaylistLoader.model = model;
                addToPlaylistPage.addItemList(model.id, chosenElements, songPlaylistLoader.item.containerUpdateID());

                mainPageStack.goBack();  // go back to the previous page
            }
        }
    }

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
        anchors {
            fill: parent
            topMargin: -units.gu(6.125)  // FIXME: 6.125 is the header.height
        }
        active: AllPlaylistsModel.count === 0 && loadedIndex
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
                popInfo.open(i18n.tr("song added"));
                return true;
            }
            popInfo.open(i18n.tr("Action can't be performed"));
            return false;
        }
    }
}
