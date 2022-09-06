/*
 * Copyright (C) 2022
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
import "components/ListItemActions"

MusicPage {
    id: radiosPage
    objectName: "radiostationsPage"
    pageTitle: qsTr("My Radios")
    pageFlickable: radioList
    searchable: true
    addVisible: true
    isListView: true

    property bool changed: false
    property bool childrenChanged: false

    onSearchClicked: filter.visible = true

    header: MusicFilter {
        id: filter
        visible: false
    }

    SortFilterModel {
        id: radiosModelFilter
        model: AllRadiosModel
        sort.property: "title"
        sort.order: Qt.AscendingOrder
        sortCaseSensitivity: Qt.CaseInsensitive
        filter.property: "normalized"
        filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
        filterCaseSensitivity: Qt.CaseInsensitive
    }

    MusicListView {
        id: radioList
        anchors.fill: parent
        model: radiosModelFilter
        delegate: MusicListItem {
            id: listItem

            onSwipe: {
                radioList.focusIndex = model.index > 0 ? model.index - 1 : 0;
                delayRemoveRadio.start();
                color = "red";
            }

            property bool held: false
            onPressAndHold: held = true
            onReleased: held = false

            color: listItem.held ? "lightgrey" : "transparent"

            noCover: "qrc:/images/radio.png"
            imageSources: [{art: "qrc:/images/radio.png"}]
            description: ""

            onActionPressed: clickItem(model)
            actionVisible: true
            actionIconSource: "qrc:/images/media-preview-start.svg"
            menuVisible: true

            menuItems: [
                Remove {
                    onTriggered: {
                        delayRemoveRadio.start();
                        color: "red";
                    }
                }
            ]

            coverSize: units.gu(5)

            column: Column {
                Label {
                    id: radioTitle
                    color: styleMusic.view.primaryColor
                    font.pointSize: units.fs("medium")
                    text: model.title
                }
            }

            Timer {
                id: delayRemoveRadio
                interval: 100
                onTriggered: {
                    var future = Sonos.tryDestroyRadio(model.id);
                    future.finished.connect(mainView.actionFinished);
                    future.start();
                }
            }
        }

        property int focusIndex: 0

        Connections {
            target: AllRadiosModel
            onLoaded: {
                if (radioList.focusIndex > 0) {
                    radioList.positionViewAtIndex(radioList.focusIndex, ListView.Center);
                    radioList.focusIndex = 0;
                }
            }
        }
    }

    function clickItem(model) {
        player.playSource(model, mainView.actionFinished); // play it
    }

    // Overlay to show when no playlists are on the device
    Loader {
        anchors.fill: parent
        active: AllRadiosModel.dataState === Sonos.DataSynced && AllRadiosModel.count === 0
        asynchronous: true
        source: "qrc:/controls2/components/RadiosEmptyState.qml"
        visible: active
    }

    DialogNewRadio {
        id: dialogNewRadio
    }

    onAddClicked: dialogNewRadio.open()

    // Overlay to show when load failed
    Loader {
        anchors.fill: parent
        active: AllRadiosModel.failure
        asynchronous: true
        sourceComponent: Component {
            DataFailureState {
                onReloadClicked: AllRadiosModel.asyncLoad();
            }
        }
        visible: active
    }
}
