/*
 * Copyright (C) 2013, 2014, 2015, 2016, 2017
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

Item {
    id: coverGrid
    height: size
    width: size

    // Property (array) to store the cover images
    property var covers: []

    // Property to set the flow of cover arts: 1, 2, 3 or 4
    property int flowModel: 1

    // Property to set the size of the cover image
    property int size

    // Property to determine if the fallback art should be used
    property bool useFallbackArt: true

    // Property to set source of default cover image
    property string noCover: "qrc:/images/no_cover.png"

    // Property to set the visibility of noCover as overlay
    property bool overlay: true

    property string firstSource

    signal imageError(var index)

    onCoversChanged: {
        if (covers !== undefined) {
            while (covers.length > 4) {  // remove any covers after 4
                covers.pop()
            }
            imageRow.flowCount = covers.length > flowModel ? flowModel : covers.length
        }
    }

    Image {
        id: overlayImage
        asynchronous: true
        fillMode: Image.PreserveAspectCrop
        height: coverGrid.size
        width: coverGrid.size
        source: noCover
        sourceSize.height: 128
        sourceSize.width: 128
        visible: coverGrid.overlay
    }

    // Flow of the cover arts in either 1, 1x1, 2x1, 2x2
    Flow {
        id: imageRow
        anchors {
            fill: parent
        }
        property int flowCount: coverGrid.flowModel

        Repeater {
            id: repeat
            model: imageRow.flowCount === 0 ? 1 : imageRow.flowCount
            delegate: Image {
                asynchronous: true
                fillMode: Image.PreserveAspectCrop
                height: coverGrid.size / (imageRow.flowCount > 1 ? 2 : 1)
                width: coverGrid.size / (imageRow.flowCount > 2 && !(imageRow.flowCount === 3 && index === 2) ? 2 : 1)
                source: coverGrid.covers.length !== 0 && coverGrid.covers[index] !== undefined && coverGrid.covers[index].art.length > 0
                        ? coverGrid.covers[index].art : ""

                sourceSize.height: 128
                sourceSize.width: 128

                onStatusChanged: {
                    if (status === Image.Error) {
                        coverGrid.imageError(index);
                        if (coverGrid.useFallbackArt) {
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
                    NumberAnimation {}
                }
            }
        }
    }
}
