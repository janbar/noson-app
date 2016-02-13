/*
 * Copyright 2015 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import Ubuntu.Components 1.3

Item {
    id: bubbleShape

    /*!
      Do not use an UbuntuShape but a Rectangle as the background of the BubbleShape.
     */
    property bool square: false

    /*!
      The background color of the bubble.
     */
    property color color: square ? theme.palette.normal.background : theme.palette.normal.overlay

    property point target
    property string direction: "down"
    property bool clipContent: false
    default property alias children: content.children
    // FIXME: This should not be necessary. See
    // https://bugs.launchpad.net/ubuntu-ui-toolkit/+bug/1214978
    property alias arrowSource: arrow.source

    implicitWidth: units.gu(10)
    implicitHeight: units.gu(8)

    signal showCompleted()
    signal hideCompleted()

    opacity: 0.0

    function show() {
        hideAnimation.stop();
        showAnimation.start();
    }

    function hide() {
        showAnimation.stop();
        hideAnimation.start();
    }

    ParallelAnimation {
        id: showAnimation

        NumberAnimation {
            target: bubbleShape
            property: "opacity"
            from: 0.0
            to: 1.0
            duration: UbuntuAnimation.FastDuration
            easing: UbuntuAnimation.StandardEasing
        }
        NumberAnimation {
            target: scaleTransform
            property: (direction === "up" || direction === "down") ? "yScale" : "xScale"
            from: 0.91
            to: 1.0
            duration: UbuntuAnimation.FastDuration
            easing: UbuntuAnimation.StandardEasing
        }
        onStopped: showCompleted()
    }

    NumberAnimation {
        id: hideAnimation
        target: bubbleShape
        property: "opacity"
        from: 1.0
        to: 0.0
        duration: UbuntuAnimation.FastDuration
        easing: UbuntuAnimation.StandardEasing
        onStopped: hideCompleted()
    }

    transform: Scale {
        id: scaleTransform
        origin.x: direction === "right" ? bubbleShape.width :
                  direction === "left" ? 0 :
                                          bubbleShape.width/2.0
        origin.y: direction === "up" ? 0 :
                  direction === "down" ? bubbleShape.height :
                                         bubbleShape.height/2.0
    }

    BorderImage {
        id: shadow
        anchors.fill: parent
        anchors.margins: square ? -units.gu(1) : -units.dp(2)
        anchors.topMargin: square ? 0 : anchors.margins
        source: Qt.resolvedUrl("../artwork/bubble_shadow@8.sci")
        opacity: 0.8
    }

    UbuntuShape {
        anchors.fill: parent
        backgroundColor: theme.palette.normal.overlay
        source: bubbleShape.clipContent ? shapeSource : null
        visible: !square
    }

    ShaderEffectSource {
        id: shapeSource
        visible: bubbleShape.clipContent
        sourceItem: bubbleShape.clipContent ? content : null
        hideSource: !square
        // FIXME: visible: false prevents rendering so make it a nearly
        // transparent 1x1 pixel instead
        opacity: 0.01
        width: 1
        height: 1
    }

    Item {
        id: content
        anchors.fill: parent

        Rectangle {
            id: colorRect
            anchors.fill: parent
            color: bubbleShape.color
            visible: bubbleShape.clipContent
        }
    }

    Item {
        x: target.x
        y: target.y

        Image {
            id: arrow

            visible: !square && bubbleShape.direction != "none"

            function directionToRotation(direction) {
                switch (direction) {
                case "up":
                    return 180;
                case "left":
                    return 90;
                case "right":
                    return -90;
                default: // "down" or "none"
                    return 0;
                }
            }

            x: -width / 2.0
            y: -height
            transformOrigin: Item.Bottom
            rotation: directionToRotation(bubbleShape.direction)
            source: Qt.resolvedUrl("../artwork/bubble_arrow@8.png")
        }
    }
}
