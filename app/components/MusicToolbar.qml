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

Rectangle {
    anchors {
        bottom: parent.bottom
        left: parent.left
        right: parent.right
    }
    color: styleMusic.common.black
    height: units.gu(7.25)
    objectName: "musicToolbarObject"

    // Hack for autopilot otherwise MusicToolbar appears as QQuickRectangle
    // due to bug 1341671 it is required that there is a property so that
    // qml doesn't optimise using the parent type
    property bool bug1341671workaround: true

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
                text: player.currentCount === 0 ? i18n.tr("No music in queue") : i18n.tr("Tap to play music")
                fontSize: "large"
                wrapMode: Text.WordWrap
                maximumLineCount: 2
            }

            /* Play/Pause button */
            Icon {
                id: disabledPlayerControlsPlayButton
                anchors {
                    right: parent.right
                    rightMargin: units.gu(3)
                    verticalCenter: parent.verticalCenter
                }
                visible: player.currentCount > 0
                color: "white"
                height: units.gu(4)
                name: player.isPlaying ? "media-playback-pause" : "media-playback-start"
                width: height
            }

            /* Select input */
            Icon {
                id: disabledPlayerControlsSelectButton
                anchors {
                    right: parent.right
                    rightMargin: units.gu(3)
                    verticalCenter: parent.verticalCenter
                }
                visible: player.currentCount === 0
                color: "white"
                height: units.gu(4)
                source: Qt.resolvedUrl("../graphics/input.svg")
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
                        currentDialog = PopupUtils.open(Qt.resolvedUrl("../components/Dialog/DialogSelectSource.qml"), mainView)
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
                 }
                 covers: [{art: player.currentMetaArt, author: player.currentMetaArtist, album: player.currentMetaAlbum}]
                 size: parent.height
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
                    color: styleMusic.common.music
                    elide: Text.ElideRight
                    fontSize: "small"
                    font.weight: Font.DemiBold
                    text: player.currentMetaTitle === "" ? player.currentMetaURITitle : player.currentMetaTitle
                }

                /* Artist of track */
                Label {
                    id: playerControlsArtist
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    color: styleMusic.common.music
                    elide: Text.ElideRight
                    fontSize: "small"
                    opacity: 0.4
                    text: player.currentMetaArtist
                }
            }

            /* Play/Pause button */
            Icon {
                id: playerControlsPlayButton
                anchors {
                    right: parent.right
                    rightMargin: units.gu(3)
                    verticalCenter: parent.verticalCenter
                }
                color: "white"
                height: units.gu(4)
                name: player.playbackState === "PLAYING" ? "media-playback-pause" : "media-playback-start"
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
                onClicked: !mainPageStack.nowPlayingLoaded ? tabs.pushNowPlaying() : mainPageStack.goBack()
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
                    color: "white"
                    opacity: parent.pressed ? 0.1 : 0

                    Behavior on opacity {
                        UbuntuNumberAnimation {
                            duration: UbuntuAnimation.FastDuration
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
            color: styleMusic.common.black
            height: units.gu(0.25)

            Rectangle {
                id: playerControlsProgressBarHint
                anchors {
                    left: parent.left
                    top: parent.top
                }
                color: UbuntuColors.blue
                height: parent.height
                width: player.duration > 0 ? (player.position / player.duration) * playerControlsProgressBar.width : 0

                Connections {
                    target: player
                    onPositionChanged: {
                        playerControlsProgressBarHint.width = (player.position / player.duration) * playerControlsProgressBar.width
                    }
                    onStopped: {
                        playerControlsProgressBarHint.width = 0;
                    }
                }
            }
        }
    }
}
