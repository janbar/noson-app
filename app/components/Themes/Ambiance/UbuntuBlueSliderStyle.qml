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
import Ubuntu.Components 1.2

/*
  The default slider style consists of a bar and a thumb shape.

  This style is themed using the following properties:
  - thumbSpacing: spacing between the thumb and the bar
*/
Item {
    id: sliderStyle

    property color foregroundColor: UbuntuColors.blue // CUSTOM
    property color backgroundColor: styleMusic.mainView.backgroundColor // CUSTOM

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
        radius: units.gu(1.5)
        width: units.gu(1.5)
        height: units.gu(1.5)
        opacity: 0.97
        color: UbuntuColors.blue
    }

    BubbleShape {
        id: bubbleShape

        property real minimumWidth: units.gu(8)
        property real horizontalPadding: units.gu(1)

        width: Math.max(minimumWidth, label.implicitWidth + 2*horizontalPadding)
        height: units.gu(3)

        // FIXME: very temporary implementation
        property real minX: 0.0
        property real maxX: background.width - width
        property real pointerSize: units.dp(6)
        property real targetMargin: units.gu(1)
        property point globalTarget: Qt.point(thumb.x + thumb.width / 2.0, thumb.y - targetMargin)

        x: MathUtils.clamp(globalTarget.x - width / 2.0, minX, maxX)
        y: globalTarget.y - height - pointerSize
        target: Qt.point(globalTarget.x - x, globalTarget.y - y)

        property bool pressed: SliderUtils.isPressed(styledItem)
        property bool shouldShow: pressed && label.text != ""
        onShouldShowChanged: if (shouldShow) {
                                show();
                             } else {
                                hide();
                             }

        Label {
            id: label
            anchors.centerIn: parent
            text: styledItem.formatValue(SliderUtils.liveValue(styledItem))
            color: Theme.palette.normal.overlayText
        }
    }
}
