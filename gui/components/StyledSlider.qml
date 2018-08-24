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

Slider {
    id: control
    property color foregroundColor: "green"
    property color backgroundColor: "grey"
    property color handleColor: "white"
    property color handleColorPressed: "lightgrey"
    property color handleBorderColor: "darkgrey"
    property alias handleSize: thumb.size

    background: Item {
        implicitWidth: units.gu(38)
        implicitHeight: units.gu(5)

        Rectangle {
            x: control.leftPadding
            y: control.topPadding + control.availableHeight / 2 - height / 2
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                right: parent.right
            }
            width: control.availableWidth
            height: units.dp(2)
            radius: 0
            color: control.backgroundColor
            opacity: 0.4
        }

        Rectangle {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
            width: control.leftPadding + control.visualPosition * control.availableWidth
            height: units.dp(2)
            color: control.foregroundColor
            radius: 2
        }
    }

    handle: Rectangle {
        id: thumb
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        property real size: units.gu(1.5)
        implicitWidth: size
        implicitHeight: size
        radius: size / 2
        color: control.pressed ? control.handleColorPressed : control.handleColor
        border.color: control.handleBorderColor
    }
}
