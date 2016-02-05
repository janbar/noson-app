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

import QtQuick 2.4
import Ubuntu.Components 1.2
import Ubuntu.Components.Popups 1.0
import "../components"
import "../components/HeadState"

MusicPage {
    id: nowPlaying
    flickable: isListView ? queueListLoader.item : fullViewLoader.item // Ensures that the header is shown in fullview
    objectName: "nowPlayingPage"
    showToolbar: false
    title: nowPlayingTitle
    visible: false

    property bool isListView: false
    // TRANSLATORS: this appears in the header with limited space (around 20 characters)
    property string nowPlayingTitle: i18n.tr("Now playing")
    // TRANSLATORS: this appears in the header with limited space (around 20 characters)
    property string fullViewTitle: i18n.tr("Full view")
    // TRANSLATORS: this appears in the header with limited space (around 20 characters)
    property string queueTitle: i18n.tr("Queue")

    onIsListViewChanged: {
        if (isListView) {  // When changing to the queue positionAt the currentIndex
            // ensure the loader and listview is ready
            if (queueListLoader.status === Loader.Ready) {
                ensureListViewLoaded()
            } else {
                queueListLoader.onStatusChanged.connect(function() {
                    if (queueListLoader.status === Loader.Ready) {
                        ensureListViewLoaded()
                    }
                })
            }
        } else {
            // Close multiselection mode.
            if (queueListLoader.status === Loader.Ready)
                queueListLoader.item.closeSelection()
        }
    }

    // Ensure that the listview has loaded before attempting to positionAt
    function ensureListViewLoaded() {
        if (queueListLoader.item.count === player.trackQueue.model.count) {
            positionAt(player.currentIndex);
        } else {
            queueListLoader.item.onCountChanged.connect(function() {
                if (queueListLoader.item.count === player.trackQueue.model.count) {
                    positionAt(player.currentIndex);
                }
            })
        }
    }

    // Position the view at the index
    function positionAt(index) {
        queueListLoader.item.positionViewAtIndex(index, ListView.Center);
    }

    state: isListView && queueListLoader.item.state === "multiselectable" ? "selection" : "default"
    states: [
        PageHeadState {
            id: defaultState
            name: "default"
            actions: [
                Action {
                    enabled: true
                    iconName: isListView ? "view-fullscreen" : "media-playlist"
                    // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
                    text: i18n.tr("Show queue")
                    onTriggered: {
                        isListView = !isListView
                    }
                },
                Action {
                    enabled: player.trackQueue.model.count > 0
                    iconName: "settings"
                    objectName: "queueActions"
                    // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
                    text: i18n.tr("Manage queue")
                    onTriggered: {
                        currentDialog = PopupUtils.open(Qt.resolvedUrl("../components/Dialog/DialogManageQueue.qml"), mainView)
                    }
                },
                Action {
                    enabled: true
                    //iconName: "clock"
                    iconSource: Qt.resolvedUrl("../graphics/timer.svg")
                    objectName: "timerActions"
                    // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
                    text: i18n.tr("Standby timer")
                    onTriggered: {
                        currentDialog = PopupUtils.open(Qt.resolvedUrl("../components/Dialog/DialogSleepTimer.qml"), mainView)
                    }
                }
            ]
            backAction: Action {
                iconName: "back"
                onTriggered: {
                    mainPageStack.goBack()
                }
            }
            PropertyChanges {
                target: nowPlaying.head
                backAction: defaultState.backAction
                actions: defaultState.actions
            }
        },
        MultiSelectHeadState {
            id: selectionState
            name: "selection"
            listview: queueListLoader.item
            thisPage: nowPlaying
            addToQueue: false
            removable: false

            /*onRemoved: {
                // Remove the tracks from the queue
                // Use slice() to copy the list
                // so that the indexes don't change as they are removed
                removeQueueList(selectedIndices.slice())
            }*/
        }
    ]

    Loader {
        id: fullViewLoader
        anchors {
            bottomMargin: nowPlayingToolbarLoader.height
            fill: parent
        }
        source: "../components/NowPlayingFullView.qml"
        visible: !isListView
    }

    Loader {
        id: queueListLoader
        anchors {
            bottomMargin: nowPlayingToolbarLoader.height
            fill: parent
        }
        asynchronous: true
        source: "../components/Queue.qml"
        visible: isListView
    }

    Loader {
        id: nowPlayingToolbarLoader
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: units.gu(15)
        source: "../components/NowPlayingToolbar.qml"
    }
}
