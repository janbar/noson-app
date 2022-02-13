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
import QtQuick.Layouts 1.3
import "../../toolbox.js" as ToolBox

Page {
    id: nowPlayingSidebar
    anchors.fill: parent

    property bool isListView: true

    Column {
        id: toolbar
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        NowPlayingToolbar {
            id: nowPlayingToolBar
            preferedHeight: units.gu(16)
            height: toolbarHeight
            width: parent.width
            bottomProgressHint: false
            mirror: true
        }

        /* Background for progress bar component */
        Rectangle {
            id: fullviewProgressBackground
            height: units.gu(3)
            width: parent.width
            color: "transparent"

            /* Progress bar component */
            Item {
                id: fullviewProgressContainer
                anchors {
                    left: fullviewProgressBackground.left
                    right: fullviewProgressBackground.right
                    top: fullviewProgressBackground.top
                    topMargin: -units.gu(2)
                }
                height: units.gu(2)
                width: parent.width
                visible: player.isPlayingQueued()

                /* Position label */
                Label {
                    id: fullviewPositionLabel
                    anchors {
                        top: progressSliderMusic.bottom
                        topMargin: units.gu(-2)
                        left: parent.left
                        leftMargin: units.gu(1)
                    }
                    color: styleMusic.nowPlaying.secondaryColor
                    font.pointSize: units.fs("small")
                    height: parent.height
                    horizontalAlignment: Text.AlignLeft
                    text: durationToString(player.trackPosition)
                    verticalAlignment: Text.AlignVCenter
                    width: units.gu(3)
                }

                StyledSlider {
                    id: progressSliderMusic
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                    }
                    from: 0
                    to: player.trackDuration  // load value at startup
                    objectName: "progressSliderShape"
                    value: player.trackPosition  // load value at startup
                    wheelEnabled: false
                    stepSize: 5000.0

                    foregroundColor: styleMusic.playerControls.progressForegroundColor
                    backgroundColor: styleMusic.playerControls.progressBackgroundColor
                    handleColor: styleMusic.playerControls.progressHandleColor
                    handleColorPressed: styleMusic.playerControls.backgroundColor
                    handleBorderColor: handleColor
                    handleSize: units.gu(1.5)

                    function formatValue(v) {
                        if (seeking) {  // update position label while dragging
                            fullviewPositionLabel.text = durationToString(v)
                        }

                        return durationToString(v)
                    }

                    property bool seeking: false
                    property bool seeked: false

                    onSeekingChanged: {
                        if (seeking === false) {
                            fullviewPositionLabel.text = durationToString(player.trackPosition)
                        }
                    }

                    onPressedChanged: {
                        seeking = pressed

                        if (!pressed) {
                            seeked = true
                            seekInTrack.start(); // start or restart the request

                            fullviewPositionLabel.text = durationToString(value)
                        }
                    }

                    Timer {
                        id: seekInTrack
                        interval: 250
                        onTriggered: {
                            player.seek(progressSliderMusic.value, mainView.actionFinished);
                        }
                    }

                    Connections {
                        target: player
                        onCurrentPositionChanged: {
                            // seeked is a workaround for bug 1310706 as the first position after a seek is sometimes invalid (0)
                            if (progressSliderMusic.seeking === false && !progressSliderMusic.seeked) {
                                fullviewPositionLabel.text = durationToString(position)
                                fullviewDurationLabel.text = durationToString(duration)

                                progressSliderMusic.value = position
                                progressSliderMusic.to = duration
                            }

                            progressSliderMusic.seeked = false;
                        }
                        onStopped: fullviewPositionLabel.text = durationToString(0);
                    }
                }

                /* Duration label */
                Label {
                    id: fullviewDurationLabel
                    anchors {
                        top: progressSliderMusic.bottom
                        topMargin: units.gu(-2)
                        right: parent.right
                        rightMargin: units.gu(1)
                    }
                    color: styleMusic.nowPlaying.secondaryColor
                    font.pointSize: units.fs("small")
                    height: parent.height
                    horizontalAlignment: Text.AlignRight
                    text: durationToString(player.trackDuration)
                    verticalAlignment: Text.AlignVCenter
                    width: units.gu(3)
                }
            }
        }
    }

    Queue {
        id: queue
        queueModel: player.trackQueue
        anchors {
            top: toolbar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
    }

    footer: Item {
        height: units.gu(7.25)
        width: parent.width

        Rectangle {
            id: nowPlayingSideBarToolBar
            anchors.fill: parent
            color: styleMusic.playerControls.backgroundColor

            opacity: 1.0

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: units.gu(1)
                height: parent.height
                color: "transparent"

                // Page actions
                Item {
                    id: optionsMenu
                    anchors.right: parent.right
                    width: units.gu(5)
                    height: parent.height
                    visible: true

                    Icon {
                        width: units.gu(5)
                        height: width
                        anchors.centerIn: parent
                        source: "qrc:/images/contextual-menu.svg"

                        onClicked: optionsMenuPopup.open()

                        enabled: parent.visible

                        Menu {
                            id: optionsMenuPopup
                            x: parent.width - width
                            transformOrigin: Menu.TopRight

                            MenuItem {
                                visible: (queue.listview.count > 0)
                                height: (visible ? implicitHeight : 0)
                                text: qsTr("Manage queue")
                                font.pointSize: units.fs("medium")
                                onTriggered: dialogManageQueue.open()
                            }

                            MenuItem {
                                text: qsTr("Select source")
                                font.pointSize: units.fs("medium")
                                onTriggered: dialogSelectSource.open()
                            }
                        }
                    }
                }
            }
        }
    }

    // Ensure that the listview has loaded before attempting to positionAt
    function ensureListViewLoaded() {
        if (queue.listview.count === player.trackQueue.count) {
            positionAtCurrentIndex();
        } else {
            ToolBox.connectOnce(queue.listview.onCountChanged, function(){
                if (queueLoader.item.listview.count === player.trackQueue.count) {
                    positionAtCurrentIndex();
                }
            })
        }
    }

    // Position the view at the index
    function positionAtCurrentIndex() {
        customdebug("Set queue position view at " + player.currentIndex);
        queue.positionAt(player.currentIndex);
    }

   function activate() {
        ensureListViewLoaded();
        // tracking of current index
        player.onCurrentIndexChanged.connect(positionAtCurrentIndex);
   }

   function deactivate() {
        // disconnect tracking of current index
        player.onCurrentIndexChanged.disconnect(positionAtCurrentIndex);
    }

   Component.onDestruction: {
       deactivate();
   }
}
