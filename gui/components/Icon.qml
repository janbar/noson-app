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
import QtGraphicalEffects 1.0

MouseArea {
    id: area
    property alias source: icon.source
    property alias label: label
    property color color: styleMusic.view.foregroundColor
    property color pressedColor: styleMusic.view.highlightedColor
    property alias animationRunning: icon.animationRunning
    property int animationInterval: 500 // slow flashing
    property alias iconSize: icon.height
    height: units.gu(5)
    width: row.width + units.gu(2)
    enabled: true
    visible: true
    hoverEnabled: enabled && visible

    Row {
        id: row
        spacing: units.gu(0.5)
        height: parent.height
        visible: false
        anchors.verticalCenter: parent.verticalCenter

        Item {
            height: parent.height
            width: units.gu(0.5)
        }

        Image {
            id: icon
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height < units.gu(3) ? parent.height : (parent.height - units.gu(2))
            width: area.visible ? height : 0
            sourceSize.height: height
            sourceSize.width: width
            source: "qrc:/images/delete.svg"

            property bool rotationRunning: false

            property bool animationRunning: false

            onAnimationRunningChanged: {
                if (animationRunning)
                    animator.start();
                else
                    animator.stop();
            }

            Timer {
                id: animator
                interval: area.animationInterval
                repeat: true
                onTriggered: {
                    opacity = (opacity === 0.0 ? 1.0 : 0.0);
                }
                onRunningChanged: opacity = 1.0 // reset opacity on stop
            }

            Behavior on opacity {
                NumberAnimation { duration: area.animationInterval / 2 }
            }
        }

        Label {
            id: label
            anchors.verticalCenter: parent.verticalCenter
            width: area.visible ? implicitWidth : 0
        }
    }

    Rectangle {
        id: iconFill
        visible: false
        anchors.fill: row
        color: parent.pressed ? parent.pressedColor : parent.color
    }

    OpacityMask {
        anchors.fill: iconFill
        source: iconFill
        maskSource: row
    }

    Rectangle {
        id: ripple
        readonly property bool square: area.implicitWidth <= area.implicitHeight
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        clip: !square
        width: parent.width
        height: parent.height
        radius: height / 2
        color: area.color
        opacity: 0

        Behavior on opacity {
            NumberAnimation { duration: 100 }
        }
    }

    onEntered: ripple.opacity = 0.1
    onExited: ripple.opacity = 0
}
