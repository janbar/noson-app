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


/* Full toolbar */
Rectangle {
    id: musicToolbarFullContainer
    anchors {
        fill: parent
    }
    color: styleMusic.common.black

    property alias bottomProgressHint: playerControlsProgressBar.visible

    RenderingBubble {
        id: renderingBubble
    }

    /* volume slider component */
    Item {
        id: musicToolbarFullVolumeContainer
        anchors {
            left: parent.left
            leftMargin: units.gu(2)
            right: parent.right
            rightMargin: units.gu(2)
            bottom: musicToolbarFullButtonContainer.top
        }
        height: units.gu(3)
        width: parent.width

        /* Mute button */
        MouseArea {
            id: nowPlayingMuteButton
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height
            opacity: 1
            width: height
            onClicked: {
                player.toggleMuteGroup()
                player.refreshRenderingGroup()
                player.refreshRendering()
            }

            Icon {
                id: muteIcon
                height: units.gu(3)
                width: height
                anchors.centerIn: parent
                source: player.mute ? Qt.resolvedUrl("../graphics/audio_volume_muted.png") : Qt.resolvedUrl("../graphics/audio_volume.png")
                objectName: "muteShape"
                opacity: 1
            }
        }

        Slider {
            id: volumeGroupSlider
            anchors.left: nowPlayingMuteButton.right
            anchors.leftMargin: units.gu(2)
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            live: true
            minimumValue: 0
            maximumValue: 100
            objectName: "volumeGroupSliderShape"
            value: player.volumeMaster // load value at startup
            opacity: 1.0
            style: SoundSliderStyle {
                thumbRadius: units.gu(0.5)
                thumbSize: units.gu(2)
                thumbColor: styleMusic.playerControls.progressHandleColor
                backgroundColor: styleMusic.playerControls.backgroundColor
                foregroundColor: styleMusic.playerControls.progressForegroundColor
            }

            Timer {
                interval: 200
                repeat: true
                running: volumeGroupSlider.pressed
                onTriggered: {
                    if (volumeGroupSlider.value > player.volumeMaster + 20)
                        volumeGroupSlider.value = player.volumeMaster + 20
                    if (player.setVolumeGroup(volumeGroupSlider.value))
                        player.volumeMaster = Math.round(volumeGroupSlider.value)
                }
            }

            onPressedChanged: {
                if (!pressed) {
                    if (volumeGroupSlider.value > player.volumeMaster + 20)
                        volumeGroupSlider.value = player.volumeMaster + 20
                    if (player.setVolumeGroup(volumeGroupSlider.value))
                        player.volumeMaster = Math.round(volumeGroupSlider.value)
                }
                else if (pressed && player.renderingControlCount > 1)
                    renderingBubble.open(volumeGroupSlider)
            }

            Connections {
                target: player
                onVolumeMasterChanged: volumeGroupSlider.value = player.volumeMaster
            }
        }
    }

    Item {
        id: musicToolbarFullButtonContainer
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: units.gu(10)
        width: parent.width

        /* Repeat button */
        MouseArea {
            id: nowPlayingRepeatButton
            anchors.right: nowPlayingPreviousButton.left
            anchors.rightMargin: units.gu(1)
            anchors.verticalCenter: nowPlayingPlayButton.verticalCenter
            height: units.gu(6)
            opacity: player.repeat ? 1 : .4
            width: height
            onClicked: {
                var old = player.repeat
                if (player.toggleRepeat())
                    player.repeat = !old
            }
            Icon {
                id: repeatIcon
                height: units.gu(3)
                width: height
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
                name: "media-playlist-repeat"
                objectName: "repeatShape"
                opacity: player.repeat ? 1 : .4
            }
        }

        /* Previous button */
        MouseArea {
            id: nowPlayingPreviousButton
            anchors.right: nowPlayingPlayButton.left
            anchors.rightMargin: units.gu(1)
            anchors.verticalCenter: nowPlayingPlayButton.verticalCenter
            height: units.gu(6)
            opacity: player.trackQueue.model.count === 0  ? .4 : 1
            width: height
            onClicked: player.previousSong()

            Icon {
                id: nowPlayingPreviousIndicator
                height: units.gu(3)
                width: height
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
                name: "media-skip-backward"
                objectName: "previousShape"
                opacity: 1
            }
        }

        /* Play/Pause button */
        MouseArea {
            id: nowPlayingPlayButton
            anchors.centerIn: parent
            height: units.gu(6)
            width: height
            onClicked: player.toggle()

            Icon {
                id: nowPlayingPlayIndicator
                height: units.gu(6)
                width: height
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
                name: player.isPlaying ? "media-playback-pause" : "media-playback-start"
                objectName: "playShape"
            }
        }

        /* Next button */
        MouseArea {
            id: nowPlayingNextButton
            anchors.left: nowPlayingPlayButton.right
            anchors.leftMargin: units.gu(1)
            anchors.verticalCenter: nowPlayingPlayButton.verticalCenter
            height: units.gu(6)
            opacity: player.trackQueue.model.count === 0 ? .4 : 1
            width: height
            onClicked: player.nextSong()

            Icon {
                id: nowPlayingNextIndicator
                height: units.gu(3)
                width: height
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
                name: "media-skip-forward"
                objectName: "forwardShape"
                opacity: 1
            }
        }

        /* Shuffle button */
        MouseArea {
            id: nowPlayingShuffleButton
            anchors.left: nowPlayingNextButton.right
            anchors.leftMargin: units.gu(1)
            anchors.verticalCenter: nowPlayingPlayButton.verticalCenter
            height: units.gu(6)
            opacity: player.shuffle ? 1 : .4
            width: height
            onClicked: {
                var old = player.shuffle
                if (player.toggleShuffle())
                    player.shuffle = !old
            }

            Icon {
                id: shuffleIcon
                height: units.gu(3)
                width: height
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
                name: "media-playlist-shuffle"
                objectName: "shuffleShape"
                opacity: player.shuffle ? 1 : .4
            }
        }

        /* Object which provides the progress bar when in the queue */
        Rectangle {
            id: playerControlsProgressBar
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            color: styleMusic.common.black
            height: units.gu(0.25)
            visible: isListView

            Rectangle {
                id: playerControlsProgressBarHint
                anchors {
                    left: parent.left
                    bottom: parent.bottom
                }
                color: styleMusic.nowPlaying.progressForegroundColor
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
