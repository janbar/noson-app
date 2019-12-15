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

import "../components"
import "../components/Dialog"

MusicPage {
    id: nowPlaying
    objectName: "nowPlayingPage"

    pageTitle: qsTr("Now playing")
    pageFlickable: nowPlayingView.flickable
    isListView: false
    showToolbar: false

    onIsListViewChanged: {
        if (isListView) {
            listview = queueLoader.item.listview;
            // When changing to the queue positionAt the currentIndex
            // ensure the loader and listview is ready
            if (queueLoader.status === Loader.Ready) {
                ensureListViewLoaded()
            } else {
                queueLoader.onStatusChanged.connect(function() {
                    if (queueLoader.status === Loader.Ready) {
                        ensureListViewLoaded()
                    }
                })
            }
        } else {
            listview = null;
        }
    }

    // Ensure that the listview has loaded before attempting to positionAt
    function ensureListViewLoaded() {
        if (queueLoader.item.listview.count === player.trackQueue.model.count) {
            positionAt(player.currentIndex);
        } else {
            queueLoader.item.listview.onCountChanged.connect(function() {
                if (queueLoader.item.listview.count === player.trackQueue.model.count) {
                    positionAt(player.currentIndex);
                }
            })
        }
    }

    // Position the view at the index
    function positionAt(index) {
        customdebug("Set queue position view at " + index);
        queueLoader.item.listview.positionViewAtIndex(index > 0 ? index - 1 : 0, ListView.Beginning);
    }

    Item {
        id: nowPlayingView
        property alias flickable: view
        anchors {
            fill: parent
            bottomMargin: nowPlayingToolbar.height
            topMargin: units.gu(1)
        }
        NowPlayingFullView {
            id: view
            anchors.fill: parent
            backgroundColor: "transparent"
        }
        visible: opacity != 0.0
        opacity: !isListView ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 100 }
        }
    }

    NowPlayingToolbar {
        id: nowPlayingToolbar
        anchors {
            bottom: parent.bottom
            bottomMargin: isListView ? nowPlaying.height - height : footer.height
        }
        height: units.gu(14)
        width: parent.width
        backgroundColor: "transparent"

        Behavior on anchors.bottomMargin {
            NumberAnimation { duration: 100 }
        }
    }

    Loader {
        id: queueLoader
        anchors {
            top: nowPlayingToolbar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        asynchronous: true
    }

    // Page actions
/*!TODO    optionsMenuVisible: true
    optionsMenuContentItems: [
        MenuItem {
            visible: (queueLoader.status === Loader.Ready && queueLoader.item.listview.count > 0)
            height: (visible ? implicitHeight : 0)
            text: qsTr("Manage queue")
            font.pixelSize: units.fx("medium")
            onTriggered: dialogManageQueue.open()
        },
        MenuItem {
            text: qsTr("Select source")
            font.pixelSize: units.fx("medium")
            onTriggered: dialogSelectSource.open()
        }
    ]
*/

    Component.onCompleted: {
        queueLoader.setSource("qrc:/sfos/components/Queue.qml", { "backgroundColor": "transparent" })
    }
}
