/*
 * Copyright (C) 2018
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
import "components"
import "components/Delegates"
import "components/Flickables"
import "components/ListItemActions"
import "components/Dialog"


MusicPage {
    id: indexPage
    objectName: "indexPage"
    isRoot: true
    multiView: true
    searchable: false

    pageTitle: qsTr("My Index")
    pageFlickable: indexGrid.visible ? indexGrid : indexList

    BlurredBackground {
            id: blurredBackground
            height: parent.height
            art: "qrc:/images/no_cover.png"
    }

    ListModel {
      id: indexModel
    }

    property bool noIndex: false

    MusicListView {
        id: indexList
        anchors.fill: parent
        model: indexModel
        delegate: MusicListItem {
            id: listItem

            property bool held: false
            onPressAndHold: held = true
            onReleased: held = false
            onClicked: clickItem(model)

            color: listItem.held ? "lightgrey" : "transparent"
            noCover: ""
            imageSources: [{art: model.art}]

            column: Column {
                Label {
                    id: mediaTitle
                    color: styleMusic.view.primaryColor
                    font.pointSize: units.fs("medium")
                    objectName: "itemtitle"
                    text: model.title
                }
            }
        }

        visible: isListView ? !shareIndexInProgress : false
    }

    MusicGridView {
        id: indexGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(7)

        model: indexModel

        delegate: Card {
            id: favoriteCard
            height: indexGrid.cellHeight
            width: indexGrid.cellWidth
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
        if (settings.preferListView)
            isListView = true
    }

    onListViewClicked: {
        settings.preferListView = isListView
    }

    function resetIndexModel() {
        indexModel.clear();
        if (AllGenresModel.count > 0) {
            noIndex = false;
            indexModel.append({
                title: qsTr("Artists"),
                art: "qrc:/images/folder_artist.png",
                source: "qrc:/controls2/Library.qml",
                args: { "rootPath": "A:ALBUMARTIST", "rootTitle": qsTr("Artists") }
            });
            indexModel.append({
                title: qsTr("Composers"),
                art: "qrc:/images/folder_composer.png",
                source: "qrc:/controls2/Library.qml",
                args: { "rootPath": "A:COMPOSER", "rootTitle": qsTr("Composers") }
            });
            indexModel.append({
                title: qsTr("Contributing Artists"),
                art: "qrc:/images/folder_artist.png",
                source: "qrc:/controls2/Library.qml",
                args: { "rootPath": "A:ARTIST", "rootTitle": qsTr("Contributing Artists") }
            });
            indexModel.append({
                title: qsTr("Albums"),
                art: "qrc:/images/folder_album.png",
                source: "qrc:/controls2/Library.qml",
                args: { "rootPath": "A:ALBUM", "rootTitle": qsTr("Albums") }
            });
            indexModel.append({
                title: qsTr("Genres"),
                art: "qrc:/images/folder_genre.png",
                source: "qrc:/controls2/Library.qml",
                args: { "rootPath": "A:GENRE", "rootTitle": qsTr("Genres") }
            });
            indexModel.append({
                title: qsTr("Tracks"),
                art: "qrc:/images/folder_genre.png",
                source: "qrc:/controls2/Library.qml",
                args: { "rootPath": "A:TRACKS", "rootTitle": qsTr("Tracks"), "isListView": true }
            });
            indexModel.append({
                title: qsTr("Share"),
                art: "qrc:/images/folder_genre.png",
                source: "qrc:/controls2/Library.qml",
                args: { "rootPath": "S:", "rootTitle": qsTr("Share"), "isListView": true }
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
        source: "qrc:/controls2/components/IndexEmptyState.qml"
        visible: active
    }

    // Overlay to show when index is being refreshed
    Loader {
        anchors.fill: parent
        active: shareIndexInProgress
        asynchronous: true
        source: "qrc:/controls2/components/IndexUpdateState.qml"
        visible: active
    }

    function clickItem(model) {
        stackView.push(model.source, model.args);
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
