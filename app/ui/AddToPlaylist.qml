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
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/Dialog"


MusicPage {
    id: addToPlaylistPage
    objectName: "addToPlaylistPage"

    // TRANSLATORS: this appears in the header with limited space (around 20 characters)
    pageTitle: qsTr("Select playlist")
    pageFlickable: addtoPlaylistView
    searchable: true
    searchResultsCount: AllPlaylistsModel.count

    state: "selector"

    property var chosenElements: []

    onVisibleChanged: {
        // Load the playlistmodel if it hasn't loaded or is empty
        if (visible && (AllPlaylistsModel.count === 0)) {
            customdebug("AllPlaylistsModel is empty !!!")
            AllPlaylistsModel.asyncLoad()
        }
    }

    /*SortFilterModel {
            // Sorting disabled as it is incorrect on first run (due to workers?)
            // and SQL sorts the data correctly
            id: addToPlaylistModelFilter
            model: AllPlaylistsModel
            filter.property: "title"
            filter.pattern: new RegExp(searchHeader.query, "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }*/

    MusicGridView {
        id: addtoPlaylistView
        itemWidth: units.gu(12)
        heightOffset: units.gu(9.5)
        model: AllPlaylistsModel
        delegate: Card {
            id: playlist
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
        height: units.gu(6)
        width: parent.width

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
                anchors.topMargin: units.gu(1.5)
                anchors.rightMargin: units.gu(1)
                anchors.leftMargin: units.gu(1)
                height: units.gu(3)
                color: "transparent"

                Row {
                    spacing: units.gu(3)

                    Icon {
                        id: selectorFind
                        visible: true
                        source: "qrc:/images/find.svg"
                        height: units.gu(3)
                        onClicked: searchClicked()
                    }

                    Icon {
                        id: add
                        visible: true
                        source: "qrc:/images/add.svg"
                        height: units.gu(3)
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
                popInfo.open(qsTr("song added"));
                return true;
            }
            popInfo.open(qsTr("Action can't be performed"));
            return false;
        }
    }
}
