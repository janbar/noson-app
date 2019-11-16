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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0

//import "../components/Themes/Ambiance"


Flickable {
    id: fullview
    anchors {
        fill: parent
    }
    property alias backgroundColor: fullviewBackground.color
    property alias backgroundOpacity: fullviewBackground.opacity

    Rectangle {
        id: fullviewBackground
        anchors.fill: parent
        color: "transparent"
        opacity: 1.0

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
                anchors.centerIn: parent
                size: parent.height
                overlay: false

                /* @FIXME: QML binding for covers fails randomly. So bind manually the covers */
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
                radius: 10
                width: albumImageContainer.width
                height: albumImageContainer.height
                visible: false
            }
            BrightnessContrast {
                anchors.bottom: parent.bottom
                source: blurred
                brightness: 0.5-styleMusic.nowPlaying.primaryColor.hslLightness
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
                font.pointSize: units.fs("large")
                font.weight: Font.DemiBold
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
                font.pointSize: units.fs("small")
                font.weight: Font.DemiBold
                text: player.currentMetaArtist
            }
        }

        /* Detect cover art swipe */
        MouseArea {
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
        }
    }

    /* Background for progress bar component */
    Rectangle {
        id: fullviewProgressBackground
        anchors.bottom: fullviewBackground.bottom
        height: units.gu(3)
        width: parent.width
        color: fullviewBackground.color
        opacity: fullviewBackground.opacity
    }

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
                        progressSliderMusic.to = duration
                    }

                    progressSliderMusic.seeked = false;
                }
                onStopped: {
                    fullviewPositionLabel.text = durationToString(0);
                    fullviewDurationLabel.text = durationToString(0);
                }
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
