/*
 * Copyright (C) 2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Adam Pigg <adam@piggz.co.uk>
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

import QtQuick 2.2
import Sailfish.Silica 1.0

// generic page for music, could be useful for bottomedge implementation
Page {
    id: thisPage
    //anchors {
    //    bottomMargin: musicToolbar.visible ? musicToolbar.height : 0
    //}

    property string pageTitle: ""
    property Flickable pageFlickable: null
    property int searchResultsCount
    property bool isListView: false
    property SilicaListView listview: null
    property bool isRoot: true // by default this page is root
    property bool showToolbar: true // by default enable the music tool bar

    property alias multiView: viewType.visible
    property alias searchable: find.visible
    property alias selectable: selection.visible
    property alias addVisible: add.visible
    property alias optionsMenuVisible: optionsMenu.visible
    property alias selectAllVisible: selectAll.visible
    property alias selectNoneVisible: selectNone.visible
    property alias addToQueueVisible: addToQueue.visible
    property alias addToPlaylistVisible: addToPlaylist.visible
    property alias removeSelectedVisible: removeSelected.visible
    property alias footer: footerToolbar
    property alias header: pageHeader
    property alias pageMenuEnabled: pageMenu.enabled
    property alias pageMenuContent: pageMenu._contentColumn
    property alias pageMenuQuickSelect: pageMenu.quickSelect
    property alias optionsMenu: optionsMenu._contentColumn
    property alias musicToolbar: musicToolbar
    
    default property alias _content: _contentItem.data
    
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
        },
        State {
            name: "addToPlaylist"
        }

    ]

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

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            id: pageMenu
        }
        
        Column {
            id: column
            width: thisPage.width
            height: thisPage.height

            PageHeader {
                id: pageHeader
                title: pageTitle
            }
            Item {
                id: _contentItem
                width: parent.width
                height: parent.height - pageHeader.height - footerToolbar.height - (musicToolbar.visible ? musicToolbar.height : 0)
            }

            //Bottom toolbar
            Item {
                id: footerToolbar
                height: units.gu(8)
                width: parent.width

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
                            spacing: units.gu(0.5)
                            anchors.fill: parent

                            MusicIcon {
                                source: "image://theme/icon-m-media-playlists"
                                height: units.gu(5)
                                label.text: player.queueInfo
                                label.font.pixelSize: units.fx("x-small")

                                    onClicked: {
                                    var page = mainView.pageStack.currentPage;
                                    if (page.objectName === "queuePage")
                                        pageStack.pop();
                                    else if (page.pageTitle === qsTr("Now playing"))
                                        page.isListView = !page.isListView;
                                    else if (!mainView.wideAspect)
                                        pageStack.push("qrc:/silica/pages/QueueView.qml");
                                    }
                            }

                            MusicIcon {
                                source: "image://theme/icon-m-location"
                                height: units.gu(5)
                                label.text: currentZoneTag
                                label.font.pixelSize: units.fx("x-small")

                                onClicked: pageStack.push("qrc:/silica/pages/Zones.qml")
                            }

                            MusicIcon {
                                id: viewType
                                visible: false
                                source: isListView ? "image://theme/icon-m-tabs" : "image://theme/icon-m-menu"
                                height: units.gu(5)
                                onClicked: {
                                    isListView = !isListView
                                }
                            }

                            MusicIcon {
                                id: find
                                visible: false
                                source: "image://theme/icon-m-search"
                                height: units.gu(5)
                                onClicked: searchClicked()
                            }

                            MusicIcon {
                                id: selection
                                visible: false
                                source: "image://theme/icon-m-acknowledge"
                                height: units.gu(5)
                                onClicked: thisPage.state = "selection"
                            }

                            MusicIcon {
                                id: add
                                visible: false
                                source: "image://theme/icon-m-add"
                                height: units.gu(5)
                                label.text: qsTr("Add")
                                label.font.pixelSize: units.fx("x-small")
                                onClicked: addClicked()
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
                            property bool squeeze: addToQueue.visible && addToPlaylist.visible && removeSelected.visible

                            MusicIcon {
                                id: closeSelection
                                visible: true
                                source: "image://theme/icon-m-close"
                                height: units.gu(5)
                                label.text: qsTr("Close")
                                label.font.pixelSize: units.fx("x-small")
                                onClicked: {
                                    thisPage.state = "default"
                                    closeSelectionClicked()
                                }
                            }

                            MusicIcon {
                                id: selectAll
                                visible: true
                                source: "image://theme/icon-m-acknowledge"
                                height: units.gu(5)
                                label.text: parent.squeeze ? "" : qsTr("All")
                                label.font.pixelSize: units.fx("x-small")
                                onClicked: selectAllClicked()
                            }

                            MusicIcon {
                                id: selectNone
                                visible: true
                                source: "image://theme/icon-m-clear"
                                height: units.gu(5)
                                label.text: parent.squeeze ? "" : qsTr("Clear")
                                label.font.pixelSize: units.fx("x-small")
                                onClicked: selectNoneClicked()
                            }

                            MusicIcon {
                                id: addToQueue
                                visible: true
                                source: "image://theme/icon-m-media-playlists"
                                height: units.gu(5)
                                onClicked: addToQueueClicked()
                            }

                            MusicIcon {
                                id: addToPlaylist
                                visible: true
                                source: "image://theme/icon-m-add"
                                height: units.gu(5)
                                onClicked: addToPlaylistClicked()
                            }
                        }

                        MusicIcon {
                            id: removeSelected
                            anchors.right: parent.right
                            visible: true
                            source: "image://theme/icon-m-delete"
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
                        height: units.gu(5)
                        color: "transparent"

                        Row {
                            spacing: units.gu(0.5)

                            MusicIcon {
                                source: "image://theme/icon-m-media-playlists"
                                height: units.gu(5)
                                label.text: player.queueInfo
                                label.font.pixelSize: units.fx("x-small")

                                onClicked: {
                                    pageStack.pop();
                                    var page = mainView.pageStack.currentPage;
                                    if (!mainView.wideAspect && page.objectName !== "queuePage")
                                        pageStack.push("qrc:/silica/pages/QueueView.qml");
                                }
                            }

                            MusicIcon {
                                id: reload
                                visible: true
                                source: "image://theme/icon-m-reload"
                                height: units.gu(5)
                                onClicked: reloadClicked()
                            }

                            MusicIcon {
                                id: groupAll
                                visible: true
                                source: "image://theme/icon-m-acknowledge"
                                height: units.gu(5)
                                label.text: qsTr("All")
                                label.font.pixelSize: units.fx("x-small")
                                onClicked: groupAllZoneClicked()
                            }

                            MusicIcon {
                                id: group
                                visible: true
                                source: "image://theme/icon-m-accept"
                                height: units.gu(5)
                                label.text: qsTr("Done")
                                label.font.pixelSize: units.fx("x-small")
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
                        height: units.gu(5)
                        color: "transparent"

                        Row {
                            spacing: units.gu(0.5)

                            MusicIcon {
                                source: "image://theme/icon-m-location"
                                height: units.gu(5)
                                label.text: currentZoneTag
                                label.font.pixelSize: units.fx("x-small")
                            }

                            MusicIcon {
                                id: groupNoneRoom
                                visible: true
                                source: "image://theme/icon-m-clear"
                                height: units.gu(5)
                                label.text: qsTr("None")
                                label.font.pixelSize: units.fx("x-small")
                                onClicked: groupNoneRoomClicked()
                            }

                            MusicIcon {
                                id: groupRoom
                                visible: true
                                source: "image://theme/icon-m-accept"
                                height: units.gu(5)
                                label.text: qsTr("Done")
                                label.font.pixelSize: units.fx("x-small")
                                onClicked: groupRoomClicked()
                            }
                        }
                    }
                }

                Rectangle {
                    id: addToPlaylistToolbar
                    anchors.fill: parent
                    color: styleMusic.playerControls.backgroundColor

                    opacity: thisPage.state === "addToPlaylist" ? 1.0 : 0.0
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

                            MusicIcon {
                                id: playlistFind
                                source: "image://theme/icon-m-search"
                                height: units.gu(5)
                                onClicked: searchClicked()
                            }

                            MusicIcon {
                                id: playlistAdd
                                visible: true
                                source: "image://theme/icon-m-add"
                                height: units.gu(5)
                                label.text: qsTr("Add")
                                label.font.pixelSize: units.fx("x-small")
                                onClicked: addClicked()
                            }
                        }
                    }
                }
            }

            Loader {
                id: musicToolbar
                active: true
                height: visible ? units.gu(8) : 0
                anchors { // start offscreen
                    left: parent.left
                    right: parent.right
                }
                asynchronous: true
                source: "qrc:/silica/components/MusicToolbar.qml"
                visible: status === Loader.Ready && !noZone && (!wideAspect || player.currentMetaSource === "") &&
                         (showToolbar === undefined || showToolbar)
            }
        }

        //Noson options menu
        PushUpMenu {
            id: optionsMenu
            MenuItem {
                    text: qsTr("Sound Settings")
                    onClicked: {
                        dialogSoundSettings.open();
                    }
            }
            MenuItem {
                    text: qsTr("Standby Timer")
                    onClicked: {
                        dialogSleepTimer.open();
                    }
            }
            MenuItem {
                    text: qsTr("Sonos Settings")
                    onClicked: {
                        dialogSettings.open();
                    }
            }
            MenuItem {
                    text: qsTr("General Settings")
                    onClicked: {
                        dialogApplicationSettings.open();
                    }
            }
            MenuItem {
                    text: qsTr("About")
                    onClicked: {
                        dialogAbout.open();
                    }
            }
        }
    }
}
