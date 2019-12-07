/*
 * Copyright (C) 2016, 2017
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

Item {
    property string message: ""
    property alias backgroundColor: popover.color
    property alias labelColor: label.color

    y: units.gu(2)
    x: (parent.width - popover.width) / 2

    Rectangle {
        id: popover
        width: label.paintedWidth + units.gu(6)
        color: styleMusic.popover.backgroundColor
        radius: units.gu(1)
        opacity: 0.7
                

        Label {
            id: label
            text: message
            anchors.centerIn: parent
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            color: styleMusic.popover.labelColor
            font.pointSize: units.fs("small")
            font.weight: Font.Normal
        }
    }

    Timer {
        id: timer
        interval: 5000
        onTriggered: {
            popover.visible = false;
        }
    }

    function open(msgtxt) {
        message = msgtxt;
        popover.visible = true;
        timer.start()
    }

}
