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
                pageStack.pop()
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
            if (pageStack.currentPage === nowPlaying) {
                pageStack.pop();
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

    Item {
        id: nowPlayingView
        property alias flickable: view
        anchors {
            fill: parent
            bottomMargin: nowPlayingToolbar.height + units.gu(1)
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
            bottomMargin: isListView ? nowPlaying.height - height - header.height - footer.height : 0
        }
        height: toolbarHeight
        width: parent.width
        backgroundColor: "transparent"
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
    Component {
        id: menuItemComp
        MenuItem {
        }
    }

    Component.onCompleted: {
        queueLoader.setSource("qrc:/sfos/components/Queue.qml", { "backgroundColor": "transparent" })

        var newMenuItem = menuItemComp.createObject(pageMenu, {"text" : qsTr("Manage Queue") })
        newMenuItem.onClicked.connect(dialogManageQueue.open)
        newMenuItem = menuItemComp.createObject(pageMenu, {"text" : qsTr("Select Source") })
        newMenuItem.onClicked.connect(dialogSelectSource.open)
    }
}
