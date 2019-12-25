/*
 * Copyright (C) 2019
 *      Adam Pigg <adam@piggz.co.uk>
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

import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    Label {
        id: label
        anchors.centerIn: parent
        text: qsTr("noson")
    }

    Image {
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectCrop
        source: "qrc:/sfos/images/harbour-noson.svg"
        width: Math.min(parent.height, parent.width) / 2
        height: width
        sourceSize.height: height
        sourceSize.width: width
    }

    Image {
        id: backgroundImage
        anchors.centerIn: parent
        asynchronous: true
        fillMode: Image.PreserveAspectCrop
        height: parent.height
        source: player.currentMetaArt
        width: Math.max(parent.height, parent.width)
        sourceSize.height: 512
        sourceSize.width: 512
    }

    CoverActionList {
        id: coverAction

        CoverAction {
            iconSource: "image://theme/icon-cover-next"
            onTriggered: player.nextSong();
        }

        CoverAction {
            iconSource: player.isPlaying ? "image://theme/icon-cover-pause" : "image://theme/icon-cover-play"
            onTriggered: player.toggle();
        }
    }
}
