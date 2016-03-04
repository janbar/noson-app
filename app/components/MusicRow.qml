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


Row {
    height: units.gu(7)

    property alias column: columnComponent.sourceComponent
    property real coverSize: styleMusic.common.albumSize
    property var imageSource

    spacing: units.gu(2)

    Image {
        id: image
        anchors {
            verticalCenter: parent.verticalCenter
        }
        asynchronous: true
        fillMode: Image.PreserveAspectCrop
        height: width
        source: (imageSource !== undefined)
                ? (imageSource.art !== undefined && imageSource.art !== "")
                   ? imageSource.art
                   : (imageSource.album !== undefined && imageSource.artist !== undefined)
                      ? "image://albumart/artist=" + imageSource.artist + "&album=" + imageSource.album
                      : (imageSource.artist !== undefined)
                         ? "image://artistart/artist=" + imageSource.artist + "&album=undefined"
                         : Qt.resolvedUrl("../graphics/no_cover.png")
                : ""

        sourceSize.height: height
        sourceSize.width: width
        width: units.gu(6)

        onStatusChanged: {
            if (status === Image.Error) {
                source = Qt.resolvedUrl("../graphics/no_cover.png")
            }
        }
        visible: imageSource !== undefined
    }

    Loader {
        id: columnComponent
        anchors {
            verticalCenter: parent.verticalCenter
        }
        width: imageSource === undefined ? parent.width - parent.spacing
                                         : parent.width - image.width - parent.spacing

        onSourceComponentChanged: {
            for (var i=0; i < item.children.length; i++) {
                item.children[i].elide = Text.ElideRight
                item.children[i].height = units.gu(2)
                item.children[i].maximumLineCount = 1
                item.children[i].wrapMode = Text.NoWrap
                item.children[i].verticalAlignment = Text.AlignVCenter

                // binds to width so it is updated when screen size changes
                item.children[i].width = Qt.binding(function () { return width; })
            }
        }
    }
}
