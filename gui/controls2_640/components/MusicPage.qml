/*
 * Copyright (C) 2013, 2014, 2015, 2016
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Andrew Hayzen <ahayzen@gmail.com>
 *      Daniel Holm <d.holmen@gmail.com>
 *      Victor Thompson <victor.thompson@gmail.com>
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

// generic page for music, could be useful for bottomedge implementation
Page {
    id: thisPage

    property string pageTitle: ""
    property Flickable pageFlickable: null
    property int searchResultsCount
    property bool isListView: false
    property ListView listview: null
    property bool isRoot: true // by default this page is root
    property bool showToolbar: true // by default enable the music tool bar

    property alias multiView: viewType.visible
    property alias searchable: find.visible
    property alias selectable: selection.visible
    property alias addVisible: add.visible
    property alias optionsMenuVisible: optionsMenu.visible
    property alias optionsMenuContentItems: optionsMenuPopup.contentData
    property alias selectAllVisible: selectAll.visible
    property alias selectNoneVisible: selectNone.visible
    property alias addToQueueVisible: addToQueue.visible
    property alias addToPlaylistVisible: addToPlaylist.visible
    property alias removeSelectedVisible: removeSelected.visible

    state: "default"
    states: [
        State {
            name: "default"
        },
        State {
            name: "selection"
        },
        State {
            name: "zone"
        },
        State {
            name: "group"
        }
    ]

    signal listViewClicked

    signal goUpClicked // action for a non-root page
    signal searchClicked
    signal selectAllClicked
    signal selectNoneClicked
    signal closeSelectionClicked
    signal addToQueueClicked
    signal addToPlaylistClicked
    signal addClicked
    signal removeSelectedClicked

    signal reloadClicked

    // available signal for page 'zone'
    signal groupZoneClicked
    signal groupAllZoneClicked

    // available signal for page 'group'
    signal closeRoomClicked
    signal groupRoomClicked
    signal groupNoneRoomClicked

    footer: Item {
        height: units.gu(7.25)

        Rectangle {
            id: defaultToolBar
            anchors.fill: parent
            color: styleMusic.playerControls.backgroundColor
            opacity: thisPage.state === "default" ? 1.0 : 0.0
            enabled: opacity > 0

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: units.gu(1)
                anchors.rightMargin: units.gu(1)
                height: units.gu(5)
                color: "transparent"

                Row {
                    spacing: units.gu(1)

                    Icon {
                        source: "qrc:/images/media-playlist.svg"
                        height: units.gu(5)
                        label.text: player.queueInfo
                        label.font.pointSize: units.fs("x-small")

                        onClicked: {
                            var page = mainView.stackView.currentItem;
                            if (page.pageTitle === qsTr("Queue"))
                                stackView.pop();
                            else if (page.pageTitle === qsTr("Now playing"))
                                page.isListView = !page.isListView;
                            else if (!mainView.wideAspect)
                                stackView.push("qrc:/controls2/QueueView.qml");
                        }
                    }

                    Icon {
                        source: "qrc:/images/location.svg"
                        height: units.gu(5)
                        label.text: currentZoneTag
                        label.font.pointSize: units.fs("x-small")

                        onClicked: stackView.push("qrc:/controls2/Zones.qml")
                    }

                    Icon {
                        id: viewType
                        visible: false
                        source: isListView ? "qrc:/images/view-grid-symbolic.svg" : "qrc:/images/view-list-symbolic.svg"
                        height: units.gu(5)
                        onClicked: {
                            isListView = !isListView
                            listViewClicked()
                        }
                    }

                    Icon {
                        id: find
                        visible: false
                        source: "qrc:/images/find.svg"
                        height: units.gu(5)
                        onClicked: searchClicked()
                    }

                    Icon {
                        id: selection
                        visible: false
                        source: "qrc:/images/select.svg"
                        height: units.gu(5)
                        onClicked: thisPage.state = "selection"
                    }

                    Icon {
                        id: add
                        visible: false
                        source: "qrc:/images/add.svg"
                        height: units.gu(5)
                        label.text: qsTr("Add")
                        label.font.pointSize: units.fs("x-small")
                        onClicked: addClicked()
                    }
                }

                Item {
                    id: optionsMenu
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: units.gu(4)
                    height: parent.height
                    visible: false

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
                        }
                    }
                }
            }
        }

        Rectangle {
            id: selectionToolBar
            anchors.fill: parent
            color: styleMusic.playerControls.backgroundColor

            opacity: thisPage.state === "selection" ? 1.0 : 0.0
            enabled: opacity > 0
            Behavior on opacity {
                NumberAnimation { duration: 100 }
            }

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: units.gu(1)
                anchors.rightMargin: units.gu(1)
                height: units.gu(5)
                color: "transparent"

                Row {
                    spacing: units.gu(0.5)
                    property bool squeeze: !mainView.wideAspect && addToQueue.visible && addToPlaylist.visible && removeSelected.visible

                    Icon {
                        id: closeSelection
                        visible: true
                        source: "qrc:/images/close.svg"
                        height: units.gu(5)
                        label.text: qsTr("Close")
                        label.font.pointSize: units.fs("x-small")
                        onClicked: {
                            thisPage.state = "default"
                            closeSelectionClicked()
                        }
                    }

                    Icon {
                        id: selectAll
                        visible: true
                        source: "qrc:/images/select.svg"
                        height: units.gu(5)
                        label.text: parent.squeeze ? "" : qsTr("All")
                        label.font.pointSize: units.fs("x-small")
                        onClicked: selectAllClicked()
                    }

                    Icon {
                        id: selectNone
                        visible: true
                        source: "qrc:/images/select-undefined.svg"
                        height: units.gu(5)
                        label.text: parent.squeeze ? "" : qsTr("Clear")
                        label.font.pointSize: units.fs("x-small")
                        onClicked: selectNoneClicked()
                    }

                    Icon {
                        id: addToQueue
                        visible: true
                        source: "qrc:/images/add.svg"
                        height: units.gu(5)
                        onClicked: addToQueueClicked()
                    }

                    Icon {
                        id: addToPlaylist
                        visible: true
                        source: "qrc:/images/add-to-playlist.svg"
                        height: units.gu(5)
                        onClicked: addToPlaylistClicked()
                    }
                }

                Icon {
                    id: removeSelected
                    anchors.right: parent.right
                    visible: true
                    source: "qrc:/images/delete.svg"
                    height: units.gu(5)
                    onClicked: removeSelectedClicked()
                }
            }
        }

        Rectangle {
            id: zoneToolBar
            anchors.fill: parent
            color: styleMusic.playerControls.backgroundColor

            opacity: thisPage.state === "zone" ? 1.0 : 0.0
            enabled: opacity > 0
            Behavior on opacity {
                NumberAnimation { duration: 100 }
            }

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: units.gu(1)
                anchors.rightMargin: units.gu(1)
                height: units.gu(3)
                color: "transparent"

                Row {
                    spacing: units.gu(1)

                    Icon {
                        source: "qrc:/images/media-playlist.svg"
                        height: units.gu(5)
                        label.text: player.queueInfo
                        label.font.pointSize: units.fs("x-small")

                        onClicked: {
                            stackView.pop();
                            var page = mainView.stackView.currentItem;
                            if (!mainView.wideAspect && page.pageTitle !== qsTr("Queue"))
                                stackView.push("qrc:/controls2/QueueView.qml");
                        }
                    }

                    Icon {
                        id: reload
                        visible: true
                        source: "qrc:/images/reload.svg"
                        height: units.gu(5)
                        onClicked: reloadClicked()
                    }

                    Icon {
                        id: groupAll
                        visible: true
                        source: "qrc:/images/select.svg"
                        height: units.gu(5)
                        label.text: qsTr("All")
                        label.font.pointSize: units.fs("x-small")
                        onClicked: groupAllZoneClicked()
                    }

                    Icon {
                        id: group
                        visible: true
                        source: "qrc:/images/group.svg"
                        height: units.gu(5)
                        label.text: qsTr("Done")
                        label.font.pointSize: units.fs("x-small")
                        onClicked: groupZoneClicked()
                    }
                }
            }
        }

        Rectangle {
            id: groupToolBar
            anchors.fill: parent
            color: styleMusic.playerControls.backgroundColor

            opacity: thisPage.state === "group" ? 1.0 : 0.0
            enabled: opacity > 0
            Behavior on opacity {
                NumberAnimation { duration: 100 }
            }

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: units.gu(1)
                anchors.rightMargin: units.gu(1)
                height: units.gu(3)
                color: "transparent"

                Row {
                    spacing: units.gu(1)

                    Icon {
                        source: "qrc:/images/location.svg"
                        height: units.gu(5)
                        label.text: currentZoneTag
                        label.font.pointSize: units.fs("x-small")
                    }

                    Icon {
                        id: groupAllRoom
                        visible: true
                        source: "qrc:/images/select-undefined.svg"
                        height: units.gu(5)
                        label.text: qsTr("None")
                        label.font.pointSize: units.fs("x-small")
                        onClicked: groupNoneRoomClicked()
                    }

                    Icon {
                        id: groupRoom
                        visible: true
                        source: "qrc:/images/group.svg"
                        height: units.gu(5)
                        label.text: qsTr("Done")
                        label.font.pointSize: units.fs("x-small")
                        onClicked: groupRoomClicked()
                    }
                }
            }
        }
    }

}
