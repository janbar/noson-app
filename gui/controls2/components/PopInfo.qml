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

import QtQuick 2.9
import QtQuick.Controls 2.2

Item {
    property string message: ""
    property alias backgroundColor: containerLayoutBackground.color
    property alias labelColor: label.color

    y: units.gu(2)
    x: (parent.width - popover.width) / 2

    Popup {
        id: popover
        width: label.paintedWidth + units.gu(6)

        background: Rectangle {
                id: containerLayoutBackground
                anchors.fill: parent
                color: styleMusic.popover.backgroundColor
                radius: units.gu(1)
                opacity: 0.7
            }

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

        onOpened: timer.start()
    }

    Timer {
        id: timer
        interval: 3000
        onTriggered: {
            popover.close();
        }
    }

    function open(msgtxt) {
        message = msgtxt;
        popover.open();
    }

}
