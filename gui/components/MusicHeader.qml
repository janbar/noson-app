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

    property alias titleColumn: titleColumnLoader.sourceComponent
    property alias coverSources: coversImage.covers
    property alias coverFlow: coversImage.flowModel
    property alias noCover: coversImage.noCover
    property alias rightColumn: rightColumnLoader.sourceComponent
    property alias firstSource: coversImage.firstSource
    property bool isFavorite: false
    readonly property bool wideView: width > mainView.wideSongView
    readonly property real contentHeight: {
        titleColumnLoader.item.height + rightColumnLoader.item.height +
             coversImage.anchors.topMargin + coversImage.anchors.bottomMargin
    }

    CoverGrid {
        id: coversImage
        anchors {
            left: parent.left
            top: parent.top
            margins: units.gu(1)
        }
        size: wideView ? titleColumnLoader.item.height + rightColumnLoader.item.height : rightColumnLoader.item.height

        Behavior on size {
            NumberAnimation { }
        }
    }

    /* Show starred */
    Icon {
        id: starred
        anchors {
            bottom: coversImage.bottom
            right: coversImage.right
        }
        height: isFavorite ? (coversImage.size * 0.25) : 0
        width: height
        enabled: isFavorite
        source: "qrc:/images/starred.svg"
        color: styleMusic.card.labelColor
    }

    Loader {
        id: rightColumnLoader
        anchors {
            bottom: coversImage.bottom
            left: coversImage.right
            leftMargin: units.gu(2)
        }
    }

    Loader {
        id: titleColumnLoader
        anchors {
            left: parent.width > mainView.wideSongView ? coversImage.right : coversImage.left
            leftMargin: parent.width > mainView.wideSongView ? units.gu(2) : units.gu(0)
            right: parent.right
            rightMargin: parent.width > mainView.wideSongView ? units.gu(0) : units.gu(2)
            top: parent.width > mainView.wideSongView ? coversImage.top : coversImage.bottom
            topMargin: parent.width > mainView.wideSongView ? units.gu(0) : units.gu(1)
        }
    }
}
