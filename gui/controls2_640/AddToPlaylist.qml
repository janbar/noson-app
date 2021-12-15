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

import QtQuick 2.9
import QtQuick.Controls 2.2
import NosonApp 1.0
import "components"
import "components/Delegates"
import "components/Flickables"
import "components/Dialog"


MusicPage {
    id: addToPlaylistPage
    objectName: "addToPlaylistPage"

    //: this appears in the header with limited space (around 20 characters)
    pageTitle: qsTr("Select playlist")
    pageFlickable: addtoPlaylistView
    searchable: true

    state: "selector"

    property var chosenElements: []

    onVisibleChanged: {
        // Load the playlistmodel if it hasn't loaded or is empty
        if (visible && (AllPlaylistsModel.count === 0)) {
            customdebug("AllPlaylistsModel is empty !!!")
            AllPlaylistsModel.asyncLoad()
        }
    }

    onSearchClicked: filter.visible = true

    header: MusicFilter {
        id: filter
        visible: false
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

        delegate: Card {
            id: playlist
            height: addtoPlaylistView.cellHeight
            width: addtoPlaylistView.cellWidth
            coverSources: [{art: model.art}]
            primaryText: model.title
            secondaryTextVisible: false

            onClicked: {
                songPlaylistLoader.model = model;
                addToPlaylistPage.addItemList(model.id, chosenElements, songPlaylistLoader.item.containerUpdateID());
                songPlaylistLoader.sourceComponent = null;
                stackView.pop();
            }
        }
    }

    footer: Item {
        height: units.gu(7.25)

        Rectangle {
            id: selectorToolBar
            anchors.fill: parent
            color: styleMusic.playerControls.backgroundColor
            opacity: addToPlaylistPage.state === "selector" ? 1.0 : 0.0
            enabled: opacity > 0

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: units.gu(1)
                anchors.leftMargin: units.gu(1)
                anchors.rightMargin: units.gu(1)
                height: units.gu(5)
                color: "transparent"

                Row {
                    spacing: units.gu(1)

                    Icon {
                        id: selectorFind
                        visible: true
                        source: "qrc:/images/find.svg"
                        height: units.gu(5)
                        onClicked: searchClicked()
                    }

                    Icon {
                        id: add
                        visible: true
                        source: "qrc:/images/add.svg"
                        height: units.gu(5)
                        label.text: qsTr("Add")
                        label.font.pointSize: units.fs("x-small")
                        onClicked: addClicked()
                    }
                }
            }
        }
    }

    DialogNewPlaylist {
        id: dialogNewPlaylist
        onAccepted: {
            filter.visible = false; // clear current search
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
        active: AllPlaylistsModel.dataState === Sonos.DataSynced && AllPlaylistsModel.count === 0
        asynchronous: true
        source: "../components/PlaylistsEmptyState.qml"
        visible: active
    }

    // Add chosen elements
    function addItemList(playlistId, modelItems, containerUpdateID) {
        if (modelItems.length > 0) {
            if (modelItems.length === 1) {
                player.addItemToSavedQueue(playlistId, modelItems[0], containerUpdateID, function(result) {
                    if (result <= 0) {
                        mainView.actionFailed();
                    }
                });
            } else {
                var c = modelItems.length > mainView.queueBatchSize ? mainView.queueBatchSize : modelItems.length;
                var items = [];
                for (var i = 0; i < c; i++) {
                    items.push(modelItems[i].payload);
                }
                player.addMultipleItemsToSavedQueue(playlistId, items, containerUpdateID, function(result) {
                    if (result <= 0) {
                        mainView.actionFailed();
                    }
                });
            }
        }
    }
}
