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

import QtQuick 2.9
import QtGraphicalEffects 1.0

// Blurred background
Item {
    width: parent.width

    property string art: player.currentMetaArt === "" ? Qt.resolvedUrl("../images/no_cover.png") : player.currentMetaArt

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
        source: art // this has to be fixed for the default cover art to work - cant find in this dir

        // TODO: This should be investigated once http://pad.lv/1391368
        //       is resolved. Once it is, these can either be set to
        //       "height" and "width" or a property exposed via the
        //       SDK or Thumbnailer to avoid a regression caused by
        //       these hardcoded values changing in the Thumbnailer.
        //       512 is size of the "xlarge" thumbnails in pixels.
        sourceSize.height: 512
        sourceSize.width: 512

        visible: false
        width: Math.max(parent.height, parent.width)
    }

    // the blur
    FastBlur {
        id: backgroundBlur
        anchors.fill: backgroundImage
        source: backgroundImage
        radius: units.dp(42)
        opacity: 0.2
    }
    onArtChanged: {
        // TODO: This is a work around for LP:1261078 and LP:1306845. Ideally,
        //       there should be a better way of getting the blur to repaint
        backgroundBlur.cached = false
        backgroundImage.source = art
        backgroundBlur.cached = true
    }
}

