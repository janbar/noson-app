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

Item {
    id: coverGrid
    height: size
    width: size

    // Property (array) to store the cover images
    property var covers: []

    // Property to set the size of the cover image
    property int size

    // Property to determine if the fallback art should be used
    property bool useFallbackArt: true

    // Property to set source of default cover image
    property string noCover: Qt.resolvedUrl("../graphics/no_cover.png")

    property string firstSource

    onCoversChanged: {
        if (covers !== undefined) {
            while (covers.length > 4) {  // remove any covers after 4
                covers.pop()
            }
        }
    }

    // Flow of the cover arts in either 1, 1x1, 2x1, 2x2
    Flow {
        id: imageRow
        anchors {
            fill: parent
        }

        Repeater {
            id: repeat
            model: coverGrid.covers.length === 0 ? 1 : coverGrid.covers.length
            delegate: Image {
                asynchronous: true
                fillMode: Image.PreserveAspectCrop
                height: coverGrid.size / (coverGrid.covers.length > 1 ? 2 : 1)
                width: coverGrid.size / (coverGrid.covers.length > 2 && !(coverGrid.covers.length === 3 && index === 2) ? 2 : 1)
                source: coverGrid.covers.length !== 0 && coverGrid.covers[index] !== undefined && coverGrid.covers[index].art !== undefined
                        ? coverGrid.covers[index].art
                        : noCover

                // TODO: This should be investigated once http://pad.lv/1391368
                //       is resolved. Once it is, these can either be set to
                //       "height" and "width" or a property exposed via the
                //       SDK or Thumbnailer to avoid a regression caused by
                //       these hardcoded values changing in the Thumbnailer.
                //       512 is size of the "xlarge" thumbnails in pixels.
                sourceSize.height: 512
                sourceSize.width: 512

                onStatusChanged: {
                    if (status === Image.Error) {
                        if (useFallbackArt) {
                            var array = coverGrid.covers.slice(0);
                            array.splice(index,1);
                            coverGrid.covers = array;
                        }
                    } else if (status === Image.Ready && index === 0) {
                        firstSource = source;
                    }
                }
                opacity: status == Image.Ready ? 1.0 : 0.0
                Behavior on opacity {
                    UbuntuNumberAnimation {}
                }
            }
        }
    }
}
