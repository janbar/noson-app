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
import "components"
import "components/Dialog"
import "../toolbox.js" as ToolBox

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
                ToolBox.connectOnce(queueLoader.onStatusChanged , function(){
                    if (queueLoader.status === Loader.Ready) {
                        ensureListViewLoaded()
                    }
                })
            }
            // tracking of current index
            player.onCurrentIndexChanged.connect(positionAtCurrentIndex);
        } else {
            listview = null;
            // disconnect tracking of current index
            player.onCurrentIndexChanged.disconnect(positionAtCurrentIndex);
        }
    }

    Component.onDestruction: {
        // disconnect tracking of current index
        player.onCurrentIndexChanged.disconnect(positionAtCurrentIndex);
    }

    Connections {
        target: mainView
        function onWideAspectChanged() {
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
        if (queueLoader.item.listview.count === player.trackQueue.count) {
            positionAtCurrentIndex();
        } else {
            ToolBox.connectOnce(queueLoader.item.listview.onCountChanged, function(){
                if (queueLoader.item.listview.count === player.trackQueue.count) {
                    positionAtCurrentIndex();
                }
            })
        }
    }

    // Position the view at the index
    function positionAtCurrentIndex() {
        customdebug("Set queue position view at " + player.currentIndex);
        queueLoader.item.positionAt(player.currentIndex);
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
        preferedHeight: !isListView && mainView.height * 0.20 > units.gu(16) ? Math.min((mainView.height * 0.20), units.gu(28)) : units.gu(16)
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
        sourceComponent: Component {
            Queue {
                backgroundColor: "transparent"
                queueModel: player.trackQueue
            }
        }
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
}
