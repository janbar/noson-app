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

import QtQuick 2.2
import Sailfish.Silica 1.0

Slider {
    id: control
    property color foregroundColor: "green"
    property color backgroundColor: "grey"
    property color handleColor: "white"
    property color handleColorPressed: "lightgrey"
    property color handleBorderColor: "darkgrey"
    property alias handleSize: thumb.size
    property real  size: units.gu(38)

    background: Item {
        implicitWidth: (control.orientation == Qt.Horizontal ? control.size : units.gu(5))
        implicitHeight: (control.orientation == Qt.Horizontal ? units.gu(5) : control.size)

        Rectangle {
            x: (control.orientation == Qt.Horizontal ? control.leftPadding : control.leftPadding + control.availableWidth / 2)
            y: (control.orientation == Qt.Horizontal ? control.topPadding + control.availableHeight / 2 : control.topPadding)
            width: (control.orientation == Qt.Horizontal ? control.availableWidth : units.dp(2))
            height: (control.orientation == Qt.Horizontal ? units.dp(2) : control.availableHeight)
            radius: 0
            color: control.backgroundColor
            opacity: 0.4
        }

        Rectangle {
            x: (control.orientation == Qt.Horizontal ? control.leftPadding
                                                     : control.leftPadding + control.availableWidth / 2)
            y: (control.orientation == Qt.Horizontal ? control.topPadding + control.availableHeight / 2
                                                     : control.topPadding + control.visualPosition * control.availableHeight)
            width: (control.orientation == Qt.Horizontal ? control.visualPosition * control.availableWidth : units.dp(2))
            height: (control.orientation == Qt.Horizontal ? units.dp(2) : (1.0 - control.visualPosition) * control.availableHeight)
            color: control.foregroundColor
            radius: 2
        }
    }

    handle: Rectangle {
        id: thumb
        x: (control.orientation == Qt.Horizontal ? control.leftPadding + control.visualPosition * (control.availableWidth - width)
                                                 : control.leftPadding + (control.availableWidth + units.dp(2) - width) / 2)
        y: (control.orientation == Qt.Horizontal ? control.topPadding + (control.availableHeight + units.dp(2) - height) / 2
                                                 : control.topPadding + control.visualPosition * (control.availableHeight - height))
        property real size: units.gu(1.5)
        implicitWidth: size
        implicitHeight: size
        radius: size / 2
        color: control.pressed ? control.handleColorPressed : control.handleColor
        border.color: control.handleBorderColor
    }
}
