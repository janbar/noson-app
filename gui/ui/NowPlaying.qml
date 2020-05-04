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

    Connections {
        target: mainView
        onWideAspectChanged: {
            if (wideAspect)
                stackView.pop()
        }
    }

    // FIXME: workaround for when entering wideAspect coming back from a stacked page (AddToPlaylist) and the page being deleted breaks the stacked page
    onVisibleChanged: {
        if (wideAspect) {
           popWaitTimer.start()
        }
    }

    Timer {
        id: popWaitTimer
        interval: 250
        onTriggered: {
            if (stackView.currentItem === nowPlaying) {
                stackView.pop();
                mainView.nowPlayingPage = null;
            }
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

    BlurredBackground {
        id: nowPlayingBackground
        anchors.fill: parent
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
            bottomMargin: isListView ? nowPlaying.height - nowPlaying.footer.height - height : 0
        }
        preferedHeight: !isListView && mainView.height * 0.20 > units.gu(16) ? Math.min((mainView.height * 0.25), units.gu(28)) : units.gu(16)
        height: toolbarHeight
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

    DialogManageQueue {
        id: dialogManageQueue
    }

    DialogSelectSource {
        id: dialogSelectSource
    }

    // Page actions
    optionsMenuVisible: true
    optionsMenuContentItems: [
        MenuItem {
            visible: (queueLoader.status === Loader.Ready && queueLoader.item.listview.count > 0)
            height: (visible ? implicitHeight : 0)
            text: qsTr("Manage queue")
            font.pointSize: units.fs("medium")
            onTriggered: dialogManageQueue.open()
        },
        MenuItem {
            text: qsTr("Select source")
            font.pointSize: units.fs("medium")
            onTriggered: dialogSelectSource.open()
        }
    ]

    Component.onCompleted: {
        queueLoader.setSource("qrc:/components/Queue.qml", { "backgroundColor": "transparent" })
    }
}
