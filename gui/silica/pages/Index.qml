/*
 * Copyright (C) 2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Adam Pigg <adam@piggz.co.uk>
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
import "../components/ListItemActions"
import "../components/Dialog"


MusicPage {
    id: indexPage
    objectName: "indexPage"
    pageMenuEnabled: false
    isRoot: true
    multiView: true
    searchable: false

    pageTitle: qsTr("My Index")
    pageFlickable: indexGrid.visible ? indexGrid : indexList

    ListModel {
      id: indexModel
    }

    property bool noIndex: false

    MusicListView {
        id: indexList
        anchors.fill: parent
        clip: true
        model: indexModel
        delegate: MusicListItem {
            id: listItem

            onClicked: clickItem(model)

            color: listItem.held ? "lightgrey" : "transparent"
            noCover: ""
            imageSources: [{art: model.art}]

            coverSize: units.gu(5)

            column: Column {
                Label {
                    id: mediaTitle
                    color: styleMusic.view.primaryColor
                    font.pixelSize: units.fx("medium")
                    objectName: "itemtitle"
                    text: model.title
                }
            }

            Component.onCompleted: {
            }
        }

        visible: isListView ? !shareIndexInProgress : false
    }

    MusicGridView {
        id: indexGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(7)
        clip: true
        model: indexModel

        delegate: Card {
            id: favoriteCard
            primaryText: model.title
            secondaryTextVisible: false
            noCover: ""
            coverSources: [{art: model.art}]

            onClicked: clickItem(model)
            onPressAndHold: {
                // search...
            }
        }

        visible: isListView ? false : !shareIndexInProgress
    }

    Component.onCompleted: {
        if (AllGenresModel.isNew()) {
            AllGenresModel.init(Sonos, "", false);
            if (Sonos.isConnected())
                AllGenresModel.asyncLoad();
        } else {
            resetIndexModel();
        }
    }

    function resetIndexModel() {
        indexModel.clear();
        if (AllGenresModel.count > 0) {
            noIndex = false;
            indexModel.append({
                title: qsTr("Artists"),
                art: "qrc:/images/folder_artist.png",
                source: "qrc:/silica/pages/Artists.qml"
            });
            indexModel.append({
                title: qsTr("Albums"),
                art: "qrc:/images/folder_album.png",
                source: "qrc:/silica/pages/Albums.qml"
            });
            indexModel.append({
                title: qsTr("Genres"),
                art: "qrc:/images/folder_genre.png",
                source: "qrc:/silica/pages/Genres.qml"
            });
            indexModel.append({
                title: qsTr("Composers"),
                art: "qrc:/images/folder_composer.png",
                source: "qrc:/silica/pages/Composers.qml"
            });
        } else {
            noIndex = true;
        }
    }

    Connections {
        target: AllGenresModel
        onCountChanged: resetIndexModel()
    }

    // Overlay to show when no index found
    Loader {
        anchors.fill: parent
        active: noIndex && !infoLoadedIndex && !shareIndexInProgress && !AllGenresModel.failure
        asynchronous: true
        source: "qrc:/silica/components/IndexEmptyState.qml"
        visible: active
    }

    // Overlay to show when index is being refreshed
    Loader {
        anchors.fill: parent
        active: shareIndexInProgress
        asynchronous: true
        source: "qrc:/silica/components/IndexUpdateState.qml"
        visible: active
    }

    function clickItem(model) {
        pageStack.push(model.source);
    }

    // Overlay to show when load failed
    Loader {
        anchors.fill: parent
        active: AllGenresModel.failure
        asynchronous: true
        sourceComponent: Component {
            DataFailureState {
                onReloadClicked: AllGenresModel.asyncLoad();
            }
        }
        visible: active
    }
}
