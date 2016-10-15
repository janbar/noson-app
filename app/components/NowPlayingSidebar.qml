/*
 * Copyright (C) 2016
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

import QtQuick 2.4
import Ubuntu.Components 1.3
import "HeadState"
import "Dialog"

Rectangle {
    id: nowPlayingSidebar
    anchors {
        fill: parent
    }

    property bool isListView: true
    property DialogBase currentDialog

    color: styleMusic.nowPlaying.backgroundColor
    state: queue.state === "multiselectable" ? "selection" : "default"
    states: [
        QueueHeadState {
            stateName: "default"
            thisHeader {
                leadingActionBar {
                    actions: []  // hide tab bar
                    objectName: "sideLeadingActionBar"
                }
                z: 100  // put on top of content
            }
            thisPage: nowPlayingSidebar
        },
        MultiSelectHeadState {
            addToQueue: false
            listview: queue.listview
            removable: true
            thisHeader {
                z: 100  // put on top of content
            }
            thisPage: nowPlayingSidebar
        }
    ]
    property alias flickable: queue.listview  // fake normal Page
    property Item header: PageHeader {
        id: pageHeader
        flickable: queue.listview
        z: 100  // put on top of content
    }
    property Item previousHeader: null
    property string title: ""  // fake normal Page

    onHeaderChanged: {  // Copy what SDK does to parent header correctly
        if (previousHeader) {
            previousHeader.parent = null
        }

        header.parent = nowPlayingSidebar
        previousHeader = header;
    }

    Queue {
        id: queue
        anchors {
            fill: parent
            topMargin: 0
            bottomMargin: 0
        }
        clip: true
        header: Column {
            anchors {
                left: parent.left
                right: parent.right
            }
            NowPlayingFullView {
                anchors {
                    fill: undefined
                }
                clip: true
                height: units.gu(55)
                width: parent.width
            }
            NowPlayingToolbar {
                id: nowPlayingToolBar
                anchors {
                    fill: undefined
                }
                bottomProgressHint: false
                color: styleMusic.common.black
                height: units.gu(15)
                width: parent.width
            }
        }
    }

    // Ensure that the listview has loaded before attempting to positionAt
    function ensureListViewLoaded() {
        if (queue.listview.count === player.trackQueue.model.count) {
            positionAt(player.currentIndex);
        } else {
            queue.listview.onCountChanged.connect(function() {
                if (queue.listview.count === player.trackQueue.model.count) {
                    positionAt(player.currentIndex);
                }
            })
        }
    }

    // Position the view at the index
    function positionAt(index) {
        queue.listview.positionViewAtIndex(index, ListView.Center);
    }

    onVisibleChanged: {
        if (visible)
            ensureListViewLoaded();
    }
}
