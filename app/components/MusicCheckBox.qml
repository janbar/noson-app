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

CheckBox {
    id: control
    property color color: styleMusic.mainView.normalTextBaseColor
    property color controlDownColor: styleMusic.mainView.highlightedColor

    opacity: checkable ? 1.0 : 0.1

    contentItem: Text {
        rightPadding: control.indicator.width + control.spacing
        //leftPadding: control.indicator.width + control.spacing
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: control.down ? control.controlDownColor : control.color
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    indicator: Rectangle {
        implicitWidth: units.gu(2.5)
        implicitHeight: units.gu(2.5)
        x: control.width - width - control.rightPadding
        //x: control.rightPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        radius: units.dp(0)
        color: "transparent"
        border.color: control.down ? control.controlDownColor : control.color

        Rectangle {
            width: units.gu(1.5)
            height: units.gu(1.5)
            x: units.dp(4)
            y: units.dp(4)
            radius: units.dp(0)
            color: control.down ? control.controlDownColor : control.color
            visible: control.checked
        }
    }
}
