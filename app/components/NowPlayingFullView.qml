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
import Ubuntu.Thumbnailer 0.1
import "../components/Themes/Ambiance"


Flickable {
    id: fullview
    anchors {
        fill: parent
    }
    property alias color: fullviewBackground.color

    Rectangle {
        id: fullviewBackground
        anchors.fill: parent
        color: styleMusic.nowPlaying.backgroundColor
    }

    BlurredBackground {
        id: blurredBackground
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: fullviewBackground.height - musicToolbarFullProgressBackground.height

        Item {
            id: albumImageContainer
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: parent.top
            }
            height: parent.height
            width: parent.width

            CoverGrid {
                id: albumImage
                anchors.centerIn: parent
                covers: [{art: makeCoverSource(player.currentMetaArt, player.currentMetaArtist, player.currentMetaAlbum)}]
                size: parent.height
                noCover: ""
            }
        }

        Rectangle {
            id: nowPlayingWideAspectLabelsBackground
            anchors.bottom: parent.bottom
            color: fullview.color
            height: nowPlayingWideAspectTitle.lineCount === 1 ? units.gu(10) : units.gu(13)
            opacity: 0.8
            width: parent.width
        }

        /* Column for labels in wideAspect */
        Column {
            id: nowPlayingWideAspectLabels
            spacing: units.gu(1)
            anchors {
                left: parent.left
                leftMargin: units.gu(2)
                right: parent.right
                rightMargin: units.gu(2)
                top: nowPlayingWideAspectLabelsBackground.top
                topMargin: nowPlayingWideAspectTitle.lineCount === 1 ? units.gu(3) : units.gu(4)
            }

            /* Title of track */
            Label {
                id: nowPlayingWideAspectTitle
                anchors {
                    left: parent.left
                    leftMargin: units.gu(1)
                    right: parent.right
                    rightMargin: units.gu(1)
                }
                color: styleMusic.playerControls.labelColor
                elide: Text.ElideRight
                fontSize: "large"
                maximumLineCount: 2
                objectName: "playercontroltitle"
                text: player.currentMetaTitle
                wrapMode: Text.WordWrap
            }

            /* Artist of track */
            Label {
                id: nowPlayingWideAspectArtist
                anchors {
                    left: parent.left
                    leftMargin: units.gu(1)
                    right: parent.right
                    rightMargin: units.gu(1)
                }
                color: styleMusic.nowPlaying.labelSecondaryColor
                elide: Text.ElideRight
                fontSize: "small"
                text: player.currentMetaArtist
            }
        }

        /* Show sleep timer state */
        Icon {
            source: Qt.resolvedUrl("../graphics/timer.svg")
            anchors {
                top: nowPlayingWideAspectLabels.top
                right: nowPlayingWideAspectLabels.right
            }
            color: UbuntuColors.orange
            height: units.gu(3)
            width: height
            visible: player.sleepTimerEnabled
        }

        /* Detect cover art swipe */
        MouseArea {
            anchors.fill: parent
            property string direction: "None"
            property real lastX: -1

            onPressed: lastX = mouse.x

            onReleased: {
                var diff = mouse.x - lastX
                if (Math.abs(diff) < units.gu(4)) {
                    return;
                } else if (diff < 0) {
                    player.nextSong()
                } else if (diff > 0) {
                    player.previousSong()
                }
            }
        }
    }

    /* Background for progress bar component */
    Rectangle {
        id: musicToolbarFullProgressBackground
        anchors {
            bottom: fullviewBackground.bottom
            left: parent.left
            right: parent.right
        }
        height: units.gu(5)
        color: fullview.color
    }

    /* Progress bar component */
    Item {
        id: musicToolbarFullProgressContainer
        anchors {
            left: musicToolbarFullProgressBackground.left
            right: musicToolbarFullProgressBackground.right
            top: musicToolbarFullProgressBackground.top
        }
        height: units.gu(2)
        width: parent.width

        /* Position label */
        Label {
            id: musicToolbarFullPositionLabel
            anchors {
                top: progressSliderMusic.bottom
                topMargin: units.gu(-2)
                left: parent.left
                leftMargin: units.gu(0.25)
            }
            color: styleMusic.nowPlaying.labelSecondaryColor
            fontSize: "small"
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            text: durationToString(player.position)
            verticalAlignment: Text.AlignVCenter
            width: units.gu(3)
        }

        Slider {
            id: progressSliderMusic
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            maximumValue: player.duration  // load value at startup
            objectName: "progressSliderShape"
            style: UbuntuBlueSliderStyle {}
            value: player.position  // load value at startup

            function formatValue(v) {
                if (seeking) {  // update position label while dragging
                    musicToolbarFullPositionLabel.text = durationToString(v)
                }

                return durationToString(v)
            }

            property bool seeking: false
            property bool seeked: false

            onSeekingChanged: {
                if (seeking === false) {
                    musicToolbarFullPositionLabel.text = durationToString(player.position)
                }
            }

            onPressedChanged: {
                seeking = pressed

                if (!pressed) {
                    seeked = true
                    player.seek(value)

                    musicToolbarFullPositionLabel.text = durationToString(value)
                }
            }

            Connections {
                target: player
                onPositionChanged: {
                    // seeked is a workaround for bug 1310706 as the first position after a seek is sometimes invalid (0)
                    if (progressSliderMusic.seeking === false && !progressSliderMusic.seeked) {
                        musicToolbarFullPositionLabel.text = durationToString(player.position)
                        musicToolbarFullDurationLabel.text = durationToString(player.duration)

                        progressSliderMusic.value = player.position
                        progressSliderMusic.maximumValue = player.duration
                    }

                    progressSliderMusic.seeked = false;
                }
                onStopped: {
                    musicToolbarFullPositionLabel.text = durationToString(0);
                    musicToolbarFullDurationLabel.text = durationToString(0);
                }
            }
        }

        /* Duration label */
        Label {
            id: musicToolbarFullDurationLabel
            anchors {
                top: progressSliderMusic.bottom
                topMargin: units.gu(-2)
                right: parent.right
                rightMargin: units.gu(0.25)
            }
            color: styleMusic.nowPlaying.labelSecondaryColor
            fontSize: "small"
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            text: durationToString(player.duration)
            verticalAlignment: Text.AlignVCenter
            width: units.gu(3)
        }
    }
}
