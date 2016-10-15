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
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import "../components"
import "../components/HeadState"

MusicPage {
    id: nowPlaying
    flickable: isListView ? queueLoader.item.listview : fullViewLoader.item // Ensures that the header is shown in fullview
    objectName: "nowPlayingPage"
    showToolbar: false
    title: nowPlayingTitle

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
            // Close multiselection mode.
            if (queueLoader.status === Loader.Ready)
                queueLoader.item.listview.closeSelection()
        }
    }

    onVisibleChanged: {
        if (wideAspect) {
           popWaitTimer.start()
        }
    }

    Timer {  // FIXME: workaround for when entering wideAspect coming back from a stacked page (AddToPlaylist) and the page being deleted breaks the stacked page
        id: popWaitTimer
        interval: 250
        onTriggered: mainPageStack.popPage(nowPlaying);
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
        queueLoader.item.listview.positionViewAtIndex(index, ListView.Center);
    }

    function setListView(listView) {
        isListView = listView;
    }

    state: {
        if (isListView) {
            if (queueLoader.item.listview.state === "multiselectable") {
                "selection"
            } else {
                "default"
            }
        } else {
            "fullview"
        }
    }
    states: [
        QueueHeadState {
            stateName: "fullview"
            thisPage: nowPlaying
            thisHeader {
                flickable: thisPage.flickable
                extension: DefaultSections { }
            }
        },
        QueueHeadState {
            stateName: "default"
            thisPage: nowPlaying
            thisHeader {
                flickable: thisPage.flickable
                extension: DefaultSections { }
            }
        },
        MultiSelectHeadState {
            addToQueue: false
            listview: queueLoader.item.listview
            removable: true
            thisPage: nowPlaying
            thisHeader {
                extension: DefaultSections { }
            }
        }
    ]

    Loader {
        id: fullViewLoader
        anchors {
            bottomMargin: nowPlayingToolbarLoader.height + units.gu(10)
            fill: parent
        }
        source: "../components/NowPlayingFullView.qml"
        visible: opacity != 0.0
        opacity: !isListView ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

    Loader {
        id: queueLoader
        anchors {
            bottomMargin: nowPlayingToolbarLoader.height
            fill: parent
        }
        asynchronous: true
        source: "../components/Queue.qml"
        visible: opacity != 0.0
        opacity: isListView ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
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

    Connections {
        target: mainView
        onWideAspectChanged: {
            // Do not pop if not visible (eg on AddToPlaylist)
            if (wideAspect && nowPlaying.visible) {
                if (currentDialog)
                    PopupUtils.close(currentDialog);
                mainPageStack.popPage(nowPlaying);
            }
        }
    }
}
