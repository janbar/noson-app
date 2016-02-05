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
import Ubuntu.Components 1.2

/*
  The default slider style consists of a bar and a thumb shape.

  This style is themed using the following properties:
  - thumbSpacing: spacing between the thumb and the bar
*/
Item {
    id: sliderStyle

    property color foregroundColor: UbuntuColors.orange // CUSTOM
    property color backgroundColor: "#ebebeb" // CUSTOM
    property alias thumbSize: thumb.size
    property alias thumbRadius: thumb.radius
    property alias thumbColor: thumb.color

    property real thumbSpacing: units.gu(0)
    property Item bar: background
    property Item thumb: thumb

    implicitWidth: units.gu(38)
    implicitHeight: units.gu(5)

    UbuntuShapeOverlay {
        id: background
        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
            left: parent.left
        }
        height: units.dp(4)
        backgroundColor: sliderStyle.backgroundColor
        overlayColor: sliderStyle.foregroundColor
        overlayRect: Qt.application.layoutDirection == Qt.LeftToRight ?
            Qt.rect(0.0, 0.0, thumb.x / thumb.barMinusThumbWidth, 1.0) :
            Qt.rect(1.0 - (thumb.x / thumb.barMinusThumbWidth), 0.0, 1.0, 1.0)
    }

    Rectangle {
        id: thumb

        anchors {
            verticalCenter: parent.verticalCenter
            topMargin: thumbSpacing
            bottomMargin: thumbSpacing
        }

        property real barMinusThumbWidth: background.width - (thumb.width + 2.0*thumbSpacing)
        property real position: thumbSpacing + SliderUtils.normalizedValue(styledItem) * barMinusThumbWidth
        property bool pressed: SliderUtils.isPressed(styledItem)
        property bool positionReached: x == position
        x: position

        /* Enable the animation on x when pressing the slider.
           Disable it when x has reached the target position.
        */
        onPressedChanged: if (pressed) xBehavior.enabled = true;
        onPositionReachedChanged: if (positionReached) xBehavior.enabled = false;

        Behavior on x {
            id: xBehavior
            SmoothedAnimation {
                duration: UbuntuAnimation.FastDuration
            }
        }

        property real size: units.gu(1.5)
        width: size
        height: size
        radius: units.gu(1)
        opacity: 1.0
        color: sliderStyle.foregroundColor
    }
}
