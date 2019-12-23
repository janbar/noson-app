/*
 * Copyright (C) 2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Adam Pigg <adam@piggz.co.uk>
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

Item {
    id: control
    property color color: styleMusic.view.foregroundColor
    property color controlDownColor: styleMusic.view.highlightedColor
    property real zoom: 1.0
    property alias text: label.text
    property alias font: label.font
    property alias textAlignment: label.horizontalAlignment
    property bool checked: false
    property alias down: area.pressed
    property alias checkable: area.enabled

    signal clicked(bool checked)

    height: units.gu(5.0 * zoom)
    width: height
    opacity: checkable ? 1.0 : 0.1

    Text {
        id: label
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        width: parent.width - indicator.width - units.gu(2.0 * zoom)
        text: ""
        font.pixelSize: units.fx("medium")
        opacity: enabled ? 1.0 : 0.3
        color: control.down ? control.controlDownColor : control.color
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    Rectangle {
        id: indicator
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.margins: units.gu(1.0 * zoom)
        height: parent.height - units.gu(2.0 * zoom)
        width: height
        radius: height / 2
        color: "transparent"
        border.color: control.down ? control.controlDownColor : control.color

        Rectangle {
            width: units.gu(2.0 * zoom)
            height: units.gu(2.0 * zoom)
            x: units.gu(0.5 * zoom)
            y: units.gu(0.5 * zoom)
            radius: height / 2
            color: control.down ? control.controlDownColor : control.color
            visible: control.checked
        }
    }

    MouseArea {
        id: area
        anchors.fill: parent
        anchors.margins: -units.gu(1.0 * zoom)

        onClicked: {
            control.checked = !control.checked;
            control.clicked(control.checked);
        }
    }
}
