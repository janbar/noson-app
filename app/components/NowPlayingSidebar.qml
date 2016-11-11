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
    property string pageTitle: i18n.tr("Now playing") // fake normal Page
    property Item pageFlickable: null // fake normal Page
    property Item header: PageHeader {
        id: pageHeader
        flickable: null
        title: nowPlayingSidebar.pageTitle
        z: 100  // put on top of content

    }

    property Item previousHeader: null

    onHeaderChanged: {  // Copy what SDK does to parent header correctly
        if (previousHeader) {
            previousHeader.parent = null
        }

        header.parent = nowPlayingSidebar
        previousHeader = header;
    }

    BlurredBackground {
        anchors.fill: parent
    }

    Column {
        id: toolbar
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            topMargin: units.gu(6.5)
        }
        NowPlayingToolbar {
            id: nowPlayingToolBar
            anchors {
                fill: undefined
            }
            height: units.gu(14)
            width: parent.width
            bottomProgressHint: false
            mirror: true
            color: "transparent"
        }
    }

    Queue {
        id: queue
        anchors {
            top: toolbar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
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
                height: units.gu(48)
                width: parent.width
                color: "transparent"
            }
        }

        Connections {
            target: queue.listview
            onContentYChanged: {
                nowPlayingToolBar.bottomProgressHint = (queue.listview.contentY > -units.gu(0.5))
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
