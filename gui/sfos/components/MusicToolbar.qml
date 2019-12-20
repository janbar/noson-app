/*
 * Copyright (C) 2017
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

Item {
    id: toolBar
    objectName: "musicToolbarObject"

    property alias color: bg.color

    Rectangle {
        id: bg
        anchors.fill: parent
        color: styleMusic.playerControls.backgroundColor
        opacity: 1.0

        Rectangle {
            color: styleMusic.card.borderColor
            anchors.top: parent.top
            width: parent.width
            height: units.dp(1)
            opacity: 0.1
        }
    }

    /* Toolbar controls */
    Item {
        id: toolbarControls
        anchors {
            fill: parent
        }

        state: player.currentMetaSource === "" ? "disabled" : "enabled"
        states: [
            State {
                name: "disabled"
                PropertyChanges {
                    target: disabledPlayerControlsGroup
                    visible: true
                }
                PropertyChanges {
                    target: enabledPlayerControlsGroup
                    visible: false
                }
            },
            State {
                name: "enabled"
                PropertyChanges {
                    target: disabledPlayerControlsGroup
                    visible: false
                }
                PropertyChanges {
                    target: enabledPlayerControlsGroup
                    visible: true
                }
            }
        ]

        /* Disabled (empty state) controls */
        Item {
            id: disabledPlayerControlsGroup
            anchors {
                bottom: playerControlsProgressBar.top
                left: parent.left
                right: parent.right
                top: parent.top
            }

            Label {
                id: noSongsInQueueLabel
                anchors {
                    left: parent.left
                    leftMargin: units.gu(2)
                    right: disabledPlayerControlsPlayButton.left
                    rightMargin: units.gu(2)
                    verticalCenter: parent.verticalCenter
                }
                color: styleMusic.playerControls.labelColor
                text: player.currentCount === 0 ? qsTr("No music in queue") : qsTr("Tap to play music")
                font.pixelSize: units.fx("large")
                wrapMode: Text.WordWrap
                maximumLineCount: 2
            }

            /* Play/Pause button */
            MusicIcon {
                id: disabledPlayerControlsPlayButton
                anchors {
                    right: parent.right
                    rightMargin: units.gu(3)
                    verticalCenter: parent.verticalCenter
                }
                visible: player.currentCount > 0
                color: styleMusic.playerControls.foregroundColor
                height: units.gu(6)
                source: player.isPlaying ? "image://theme/icon-l-pause" : "image://theme/icon-l-play"
                width: height
            }

            /* Select input */
            MusicIcon {
                id: disabledPlayerControlsSelectButton
                anchors {
                    right: parent.right
                    rightMargin: units.gu(3)
                    verticalCenter: parent.verticalCenter
                }
                visible: player.currentCount === 0
                color: styleMusic.playerControls.foregroundColor
                height: units.gu(5)
                source: "qrc:/images/input.svg"
                width: height
            }

            /* Click action */
            MouseArea {
                anchors {
                    fill: parent
                }
                onClicked: {
                    if (player.currentCount > 0)
                        player.playQueue(true)
                    else
                        dialogSelectSource.open()
                }
            }
        }

        /* Enabled controls */
        Item {
            id: enabledPlayerControlsGroup
            anchors {
                bottom: playerControlsProgressBar.top
                left: parent.left
                right: parent.right
                top: parent.top
            }

            /* Album art in player controls */
            CoverGrid {
                 id:  playerControlsImage
                 anchors {
                     bottom: parent.bottom
                     left: parent.left
                     top: parent.top
                     margins: units.dp(2)
                 }
                 size: parent.height
                 overlay: false

                 Component.onCompleted: {
                     covers = player.covers.slice();
                 }
                 Connections {
                     target: player
                     onSourceChanged: {
                         playerControlsImage.covers = player.covers.slice();
                     }
                 }
            }

            /* Column of meta labels */
            Column {
                id: playerControlsLabels
                anchors {
                    left: playerControlsImage.right
                    leftMargin: units.gu(1.5)
                    right: playerControlsPlayButton.left
                    rightMargin: units.gu(1)
                    verticalCenter: parent.verticalCenter
                }

                /* Title of track */
                Label {
                    id: playerControlsTitle
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    color: styleMusic.view.primaryColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("medium")
                    font.weight: Font.DemiBold
                    text: player.currentMetaTitle
                }

                /* Artist of track */
                Label {
                    id: playerControlsArtist
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    color: styleMusic.view.secondaryColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("small")
                    text: player.currentMetaArtist
                }
            }

            /* Play/Pause button */
            MusicIcon {
                id: playerControlsPlayButton
                anchors {
                    right: parent.right
                    rightMargin: units.gu(3)
                    verticalCenter: parent.verticalCenter
                }
                color: styleMusic.playerControls.foregroundColor
                height: units.gu(6)
                source: player.playbackState === "PLAYING" ? "image://theme/icon-l-pause" : "image://theme/icon-l-play"
                objectName: "playShape"
                width: height
            }

            /* Mouse area to jump to now playing */
            MouseArea {
                anchors {
                    fill: parent
                }
                objectName: "jumpNowPlaying"
                enabled: true
                onClicked: {
                    if (mainView.nowPlayingPage !== null) {
                        //!FIXME: doPop:574 - Warning: cannot pop while transition is in progress
                        //while (pageStack.depth > 1 && pageStack.currentPage !== mainView.nowPlayingPage) {
                        //    pageStack.pop();
                        //}
                        pageStack.pop();
                    } else {
                        tabs.pushNowPlaying();
                    }
                }
            }

            /* Mouse area for the play button (ontop of the jump to now playing) */
            MouseArea {
                anchors {
                    bottom: parent.bottom
                    horizontalCenter: playerControlsPlayButton.horizontalCenter
                    top: parent.top
                }
                onClicked: player.toggle()
                width: units.gu(8)

                Rectangle {
                    anchors {
                        fill: parent
                    }
                    color: styleMusic.playerControls.foregroundColor
                    opacity: parent.pressed ? 0.1 : 0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 250
                        }
                    }
                }
            }
        }

        /* Object which provides the progress bar when toolbar is minimized */
        Rectangle {
            id: playerControlsProgressBar
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            color: styleMusic.playerControls.backgroundColor
            height: units.gu(0.25)

            Rectangle {
                id: playerControlsProgressBarHint
                anchors {
                    left: parent.left
                    top: parent.top
                }
                color: styleMusic.playerControls.progressForegroundColor
                height: parent.height
                width: hint(player.trackPosition, player.trackDuration)

                function hint(position, duration) {
                   var val = 0;
                   if (duration > 0)
                       val = playerControlsProgressBar.width * position / duration;
                   playerControlsProgressBarHint.width = val;
                }

                Connections {
                    target: player
                     onCurrentPositionChanged: playerControlsProgressBarHint.hint(position, duration)
                     onStopped: playerControlsProgressBarHint.hint(0, 0)
                }
            }
        }
    }
}
