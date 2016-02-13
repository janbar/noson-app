/*
 * Copyright (C) 2014, 2015
 *      Andrew Hayzen <ahayzen@gmail.com>
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
    width: parent.width

    property alias bottomColumn: bottomColumnLoader.sourceComponent
    property alias coverSources: coversImage.covers
    property alias rightColumn: rightColumnLoader.sourceComponent

    BlurredBackground {
        id: blurredBackground
        height: parent.height
        art: coversImage.firstSource
    }

    CoverGrid {
        id: coversImage
        anchors {
            bottomMargin: units.gu(2)
            left: parent.left
            leftMargin: units.gu(2)
            rightMargin: units.gu(2)
            top: parent.top
            topMargin: units.gu(3)
        }
        size: parent.width > units.gu(60) ? units.gu(27.5) : (parent.width - units.gu(5)) / 2
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
        id: bottomColumnLoader
        anchors {
            left: parent.width > units.gu(60) ? coversImage.right : coversImage.left
            leftMargin: parent.width > units.gu(60) ? units.gu(2) : units.gu(0)
            right: parent.right
            rightMargin: parent.width > units.gu(60) ? units.gu(0) : units.gu(2)
            top: parent.width > units.gu(60) ? coversImage.top : coversImage.bottom
            topMargin: parent.width > units.gu(60) ? units.gu(0) : units.gu(1)
        }
    }
}
