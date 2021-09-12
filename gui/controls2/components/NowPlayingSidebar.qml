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
import QtQuick.Layouts 1.3

Page {
    id: nowPlayingSidebar
    anchors.fill: parent

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
            preferedHeight: units.gu(16)
            height: toolbarHeight
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
                height: width + units.gu(6)
                width: parent.width
            }

            Item {
                width: queue.width
                height: 20
            }
        }
    }

    footer: Item {
        height: units.gu(7.25)
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
                                font.pointSize: units.fs("medium")
                                onTriggered: dialogManageQueue.open()
                            }

                            MenuItem {
                                text: qsTr("Select source")
                                font.pointSize: units.fs("medium")
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
    }
}
