/*
 * Copyright (C) 2016-2019
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
import QtQuick.Layouts 1.1

Page {
    id: nowPlayingSidebar
    anchors.fill: parent

    // under construction
    Item {
        anchors {
            fill: parent
        }

        Rectangle {
            id: notImplemented
            anchors {
                fill: parent
            }
            color: "transparent"

            Column {
                anchors.centerIn: parent
                spacing: units.gu(4)
                width: units.gu(36)

                Label {
                    color: styleMusic.view.labelColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("x-large")
                    horizontalAlignment: Text.AlignHCenter
                    maximumLineCount: 2
                    text: qsTr("Under construction")
                    width: parent.width
                    wrapMode: Text.WordWrap
                }

                Label {
                    color: styleMusic.view.foregroundColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("medium")
                    horizontalAlignment: Text.AlignLeft
                    maximumLineCount: 8
                    text: qsTr("This feature will be added in a next version. Please rate the App.")
                    width: parent.width
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

/*!TODO
    property bool isListView: true

    Column {
        id: toolbar
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        NowPlayingToolbar {
            id: nowPlayingToolBar
            anchors {
                fill: undefined
            }
            height: units.gu(14)
            width: parent.width
            bottomProgressHint: false
            mirror: true
        }
    }

    Queue {
        id: queue
        anchors {
            top: toolbar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        header: Column {
            anchors {
                left: parent.left
                right: parent.right
            }
            NowPlayingFullView {
                id: fullView
                anchors {
                    fill: undefined
                }
                clip: true
                height: width + units.gu(3)
                width: parent.width
            }
        }
    }

    footer: Item {
        height: units.gu(8)
        width: parent.width

        Rectangle {
            id: nowPlayingSideBarToolBar
            anchors.fill: parent
            color: styleMusic.playerControls.backgroundColor

            opacity: 1.0

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: units.gu(1)
                height: parent.height
                color: "transparent"

                // Page actions
                Item {
                    id: optionsMenu
                    anchors.right: parent.right
                    width: units.gu(5)
                    height: parent.height
                    visible: true

                    Icon {
                        width: units.gu(5)
                        height: width
                        anchors.centerIn: parent
                        source: "qrc:/images/contextual-menu.svg"

                        onClicked: optionsMenuPopup.open()

                        enabled: parent.visible

                        Menu {
                            id: optionsMenuPopup
                            x: parent.width - width
                            transformOrigin: Menu.TopRight

                            MenuItem {
                                visible: (queue.listview.count > 0)
                                height: (visible ? implicitHeight : 0)
                                text: qsTr("Manage queue")
                                font.pixelSize: units.fx("medium")
                                onTriggered: dialogManageQueue.open()
                            }

                            MenuItem {
                                text: qsTr("Select source")
                                font.pixelSize: units.fx("medium")
                                onTriggered: dialogSelectSource.open()
                            }
                        }
                    }
                }
            }
        }
    }

    // Ensure that the listview has loaded before attempting to positionAt
    function ensureListViewLoaded() {
        if (queue.listview.count === player.trackQueue.model.count) {
            positionAt(player.currentIndex);
        } else {
            queue.listview.onCountChanged.connect(function() {
                if (queueLoader.item.listview.count === player.trackQueue.model.count) {
                    positionAt(player.currentIndex);
                }
            })
        }
    }

    // Position the view at the index
    function positionAt(index) {
        customdebug("Set queue position view at " + index);
        queue.listview.positionViewAtIndex(index > 0 ? index - 1 : 0, ListView.Beginning);
    }

    onVisibleChanged: {
        if (visible)
            ensureListViewLoaded();
    }*/
}
