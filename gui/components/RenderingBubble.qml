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

import QtQuick 2.8
import QtQuick.Controls 2.1


Item {
    id: renderingBubble
    anchors {
        left: parent.left
        right: parent.right
        margins: units.gu(2)
    }
    property bool opened: false
    property color backgroundColor: styleMusic.popover.backgroundColor
    property color foregroundColor: styleMusic.popover.foregroundColor
    property color labelColor: styleMusic.popover.labelColor
    readonly property bool displayable: containerLayout.height >= containerLayout.rowHeight

    Popup {
        id: popover
        width: parent.width
        height: containerLayout.height
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

        background: Rectangle {
                id: containerLayoutBackground
                anchors.fill: parent
                color: renderingBubble.backgroundColor
                radius: units.gu(1)
                opacity: 0.9
            }

        Item {
            id: containerLayout
            anchors.centerIn: parent
            readonly property double rowHeight: units.gu(7)
            readonly property double minHeight: rowHeight + units.gu(4)
            readonly property double maxHeight: mainView.height - /*header-footer*/units.gu(15)
            property int contentHeight: player.renderingModel.count * rowHeight + /*margins*/units.gu(6)
            width: parent.width
            height: contentHeight > maxHeight ? maxHeight : contentHeight

            RenderingControlerView {
                id: renderingControlerView
                backgroundColor: "transparent"
                foregroundColor: renderingBubble.foregroundColor
                labelColor: renderingBubble.labelColor
                anchors.fill: parent
                anchors.topMargin: units.gu(2)
                anchors.bottomMargin: units.gu(2)
            }

            Connections {
                target: renderingControlerView
                onFinger: {
                    if (isHeld && bubbleTimer.running)
                        timer.stop()
                    else if (!isHeld)
                        timer.restart()
                }
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 500 }
        }

        onOpened: timer.start()
    }

    Timer {
        id: timer
        interval: 5000
        onTriggered: {
            opened = false
            popover.close()
        }
    }

    Connections {
        target: popover
        onClosed: {
            if (renderingBubble.opened) {
                renderingBubble.opened = false
                timer.stop()
            }
        }
    }

    function open(caller) {
        if (!renderingBubble.opened && containerLayout.height > containerLayout.minHeight) {
            popover.parent = caller;
            var gc = renderingBubble.parent.mapToItem(null, 0, 0)
            if (gc.y > (mainView.height - mainView.header.height) / 2) {
                popover.y = - (popover.height + units.gu(2))
            } else {
                popover.y = parent.height + units.gu(2)
            }
            player.refreshRendering()
            renderingBubble.opened = true
            popover.open()
        }
    }
}
