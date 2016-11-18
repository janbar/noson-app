/*
 * Copyright (C) 2016
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

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3

Item {
    property string message: ""

    Rectangle {
        id: popCaller
        color: "transparent"
        anchors {
            top: parent.bottom
            topMargin: -units.gu(1)
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: units.gu(8)
    }

    Component {
        id: popoverComponent
        Popover {
            id: popover
            width: label.paintedWidth + units.gu(6)

            Rectangle {
                id: containerLayoutBackground
                anchors.fill: parent
                color: styleMusic.popover.backgroundColor
            }

            Label {
                id: label
                text: message
                height: units.gu(4)
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: styleMusic.popover.labelColor
                fontSize: "small"
                font.weight: Font.Normal
            }

            Timer {
                id: timer
                interval: 1500

                Component.onCompleted: start();

                onTriggered: {
                    stop();
                    PopupUtils.close(popover);
                }
            }
        }
    }

    function open(msgtxt) {
        message = msgtxt;
        PopupUtils.open(popoverComponent, popCaller);
    }
}
