/*
 * Copyright (C) 2013-2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Adam Pigg <adam@piggz.co.uk>
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

import QtQuick 2.2
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0

//import "../components/Themes/Ambiance"


SilicaFlickable {
    id: fullview

    property alias backgroundColor: fullviewBackground.color

    Rectangle {
        id: fullviewBackground
        anchors.fill: parent
        color: "transparent"

        Item {
            id: albumImageContainer
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: parent.top
            }
            height: parent.height - fullviewProgressBackground.height
            width: parent.width

            CoverGrid {
                id: albumImage
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                size: Math.min(parent.height, parent.width)
                overlay: false

                Component.onCompleted: {
                    covers = player.covers.slice();
                }
                Connections {
                    target: player
                    onSourceChanged: {
                        albumImage.covers = player.covers.slice();
                    }
                }
            }
        }

        Item {
            id: nowPlayingWideAspectLabelsBackground
            anchors.bottom: albumImageContainer.bottom
            height: nowPlayingWideAspectTitle.lineCount === 1 ? units.gu(10) : units.gu(13)
            width: parent.width

            clip: true

            FastBlur {
                id: blurred
                source: albumImageContainer
                radius: 20
                width: albumImageContainer.width
                height: albumImageContainer.height
                visible: false
            }
            BrightnessContrast {
                anchors.bottom: parent.bottom
                source: blurred
                /*!FIXME: brightness: 0.5-styleMusic.nowPlaying.primaryColor.hslLightness*/
                brightness: Qt.colorEqual(styleMusic.nowPlaying.primaryColor, "black") ? +0.5 : -0.5
                width: albumImageContainer.width
                height: albumImageContainer.height
            }
        }



        /* Column for labels */
        Column {
            id: nowPlayingWideAspectLabels
            spacing: units.gu(1)
            anchors {
                left: parent.left
                leftMargin: units.gu(2)
                right: parent.right
                rightMargin: units.gu(2)
                top: nowPlayingWideAspectLabelsBackground.top
                topMargin: units.gu(2)
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
                color: styleMusic.nowPlaying.primaryColor
                elide: Text.ElideRight
                font.pixelSize: units.fx("large")
                font.weight: Font.Bold
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
                color: styleMusic.nowPlaying.primaryColor
                elide: Text.ElideRight
                font.pixelSize: units.fx("small")
                font.weight: Font.Bold
                text: player.currentMetaArtist
            }
        }

        /* Detect cover art swipe */
        /*MouseArea {
            anchors.fill: parent
            property string direction: "None"
            property real lastX: -1
            property real lastY: -1

            onPressed: {
                lastX = mouse.x
                lastY = mouse.y
            }

            onReleased: {
                var diffX = mouse.x - lastX
                if (Math.abs(diffX) > units.gu(10)) {
                    if (diffX < 0) {
                        player.nextSong()
                    } else {
                        player.previousSong()
                    }
                }
                var diffY = mouse.y - lastY
                if (Math.abs(diffY) > units.gu(5)) {
                    if (diffY < 0 && mainView.nowPlayingPage !== null) {
                        mainView.nowPlayingPage.isListView = true;
                    }
                }
            }
        }*/
    }

    /* Background for progress bar component */
    Rectangle {
        id: fullviewProgressBackground
        anchors.bottom: fullviewBackground.bottom
        height: units.gu(3)
        width: parent.width
        color: fullviewBackground.color
    }

    /* Progress bar component */
    Item {
        id: fullviewProgressContainer
        anchors {
            left: fullviewProgressBackground.left
            right: fullviewProgressBackground.right
            top: fullviewProgressBackground.top
            topMargin: -units.gu(1)
        }
        height: units.gu(2)
        width: parent.width
        visible: player.isPlayingQueued()

        /* Position label */
        Label {
            id: fullviewPositionLabel
            anchors {
                top: progressSliderMusic.bottom
                topMargin: units.gu(-4)
                left: parent.left
                leftMargin: units.gu(1)
            }
            color: styleMusic.nowPlaying.secondaryColor
            font.pixelSize: units.fx("small")
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
                leftMargin: units.gu(4)
                right: parent.right
                rightMargin: units.gu(4)
                top: parent.top
            }
            handleVisible: false
            maximumValue: player.trackDuration  // load value at startup
            objectName: "progressSliderShape"
            value: player.trackPosition  // load value at startup
            stepSize: 5000.0

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

            onDownChanged: {
                seeking = down

                if (!down) {
                    seeked = true
                    seekInTrack.start(); // start or restart the request

                    fullviewPositionLabel.text = durationToString(value)
                }
            }

            Timer {
                id: seekInTrack
                interval: 250
                onTriggered: {
                    player.seek(progressSliderMusic.value);
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
                        progressSliderMusic.maximumValue = duration
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
                topMargin: units.gu(-4)
                right: parent.right
                rightMargin: units.gu(1)
            }
            color: styleMusic.nowPlaying.secondaryColor
            font.pixelSize: units.fx("small")
            height: parent.height
            horizontalAlignment: Text.AlignRight
            text: durationToString(player.trackDuration)
            verticalAlignment: Text.AlignVCenter
            width: units.gu(3)
        }
    }
}
