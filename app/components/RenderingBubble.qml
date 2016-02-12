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
    property bool opened: false

    Component {
        id: popoverComponent
        Popover {
            id: popover
            autoClose: false
            callerMargin: units.gu(0)

            Item {
                id: containerLayout
                anchors {
                    left: parent.left
                    right: parent.right
                }
                readonly property double rowHeight: units.gu(7)
                readonly property double maxHeight: mainView.height - /*footer*/units.gu(15)
                property int contentHeight: player.renderingModel.count * rowHeight + /*margins*/units.gu(4)
                height: contentHeight > maxHeight ? maxHeight : contentHeight

                Rectangle {
                    id: containerLayoutBackground
                    anchors.fill: parent
                    color: styleMusic.popover.backgroundColor
                }

                RenderingControlerView {
                    id: renderingControlerView
                    backgroundColor: styleMusic.popover.backgroundColor
                    labelColor: styleMusic.popover.labelColor
                    anchors.topMargin: units.gu(2)
                }

                Connections {
                    target: renderingControlerView
                    onFinger: {
                        if (isHeld && bubbleTimer.running)
                            bubbleTimer.stop()
                        else if (!isHeld)
                            bubbleTimer.restart()
                    }
                }
            }

            Behavior on opacity {
                NumberAnimation { duration: 500 }
            }

            Timer {
                id: bubbleTimer
                interval: 5000

                Component.onCompleted: start()

                onTriggered: {
                    opened = false
                    popover.opacity = 0
                    closingBubbleTimer.start()
                }
            }

            Timer {
                id: closingBubbleTimer
                interval: 500

                onTriggered: {
                    popover.visible = false
                    PopupUtils.close(popover)
                }
            }
        }
    }

    function open(caller) {
        if (!opened) {
            player.refreshRendering()
            opened = true
            PopupUtils.open(popoverComponent, caller)
        }
    }
}
