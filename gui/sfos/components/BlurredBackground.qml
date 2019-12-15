/*
 * Copyright (C) 2013, 2014, 2015
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

// Blurred background
Item {
    width: parent.width

    property string art: player.currentMetaArt === "" ? "qrc:/images/no_cover.png" : player.currentMetaArt

    // dark layer
    Rectangle {
        anchors {
            fill: parent
        }
        color: styleMusic.playerControls.backgroundColor
    }

    // the album art
    Image {
        id: backgroundImage
        anchors.fill: parent
        asynchronous: true
        fillMode: Image.PreserveAspectCrop
        height: parent.height
        source: art
        visible: false
        width: Math.max(parent.height, parent.width)
        sourceSize.height: 512
        sourceSize.width: 512
    }

    // the blur
    FastBlur {
        id: backgroundBlur
        anchors.fill: backgroundImage
        source: backgroundImage
        radius: units.dp(64)
        opacity: 0.2
        cached: false
        transparentBorder: true
    }
    onArtChanged: {
        backgroundImage.source = art
    }
}

