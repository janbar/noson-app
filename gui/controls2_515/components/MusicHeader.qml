/*
 * Copyright (C) 2017
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
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

Item {
    anchors {
        left: parent.left
        right: parent.right
    }

    property alias coverSources: coversImage.covers
    property alias coverFlow: coversImage.flowModel
    property alias noCover: coversImage.noCover
    property alias rightColumn: rightColumnLoader.sourceComponent
    property alias firstSource: coversImage.firstSource
    property bool isFavorite: false
    readonly property real contentHeight: rightColumnLoader.item.height +
                                          coversImage.anchors.topMargin +
                                          coversImage.anchors.bottomMargin

    CoverGrid {
        id: coversImage
        anchors {
            left: parent.left
            top: parent.top
            margins: units.gu(1)
        }
        size: rightColumnLoader.item.height
    }

    /* Show starred */
    Icon {
        id: starred
        anchors {
            bottom: coversImage.bottom
            right: coversImage.right
        }
        height: isFavorite ? (coversImage.size * 0.40) : 0
        width: height
        enabled: isFavorite
        source: "qrc:/images/starred.svg"
        color: styleMusic.card.foregroundColor
    }

    Loader {
        id: rightColumnLoader
        anchors {
            bottom: coversImage.bottom
            left: coversImage.right
            leftMargin: units.gu(2)
        }
    }
}
