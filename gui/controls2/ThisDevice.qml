/*
 * Copyright (C) 2019
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
import NosonMediaScanner 1.0
import "components"
import "components/Delegates"
import "components/Flickables"
import "components/ListItemActions"
import "components/Dialog"


MusicPage {
    id: thisDevicePage
    objectName: "thisDevicePage"
    isRoot: true
    multiView: true
    searchable: false

    pageTitle: qsTr("This Device")
    pageFlickable: indexGrid.visible ? indexGrid : indexList

    BlurredBackground {
            id: blurredBackground
            height: parent.height
            art: "qrc:/images/no_cover.png"
    }

    ListModel {
      id: indexModel
      Component.onCompleted: {
          indexModel.append({
              title: qsTr("Artists"),
              art: "qrc:/images/folder_artist.png",
              source: "qrc:/controls2/ThisDevice/Artists.qml"
          });
          indexModel.append({
              title: qsTr("Albums"),
              art: "qrc:/images/folder_album.png",
              source: "qrc:/controls2/ThisDevice/Albums.qml"
          });
          indexModel.append({
              title: qsTr("Genres"),
              art: "qrc:/images/folder_genre.png",
              source: "qrc:/controls2/ThisDevice/Genres.qml"
          });
      }
    }

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

        visible: isListView ? !MediaScanner.emptyState : false
    }

    MusicGridView {
        id: indexGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(7)

        model: indexModel

        delegate: Card {
            id: favoriteCard
            primaryText: model.title
            secondaryTextVisible: false
            noCover: ""
            coverSources: [{art: model.art}]

            onClicked: clickItem(model)
        }

        visible: isListView ? false : !MediaScanner.emptyState
    }

    GenreList {
        id: genres
    }

    Component.onCompleted: {
        genres.init();
        MediaScanner.start();
    }

    // Overlay to show when no index found
    Loader {
        anchors.fill: parent
        active: MediaScanner.emptyState
        asynchronous: true
        source: "qrc:/controls2/components/IndexEmptyState.qml"
        visible: active
    }

    function clickItem(model) {
        stackView.push(model.source, { "pageTitle": pageTitle });
    }
}
