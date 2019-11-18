/*
 * Copyright (C) 2013, 2014, 2015, 2016, 2017
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
import "ListItemActions"

Row {
    id: row
    anchors.leftMargin: units.gu(1)
    anchors.rightMargin: units.gu(2)

    property real contentHeight: units.gu(6)
    property alias column: columnComponent.sourceComponent
    property real coverSize: contentHeight
    property string noCover: "qrc:/images/no_cover.png"
    property var imageSources: []
    property string imageSource: ""
    property string description: ""
    property bool isFavorite: false
    property alias checked: select.checked
    property alias checkable: select.checkable

    property bool menuVisible: false
    property alias menuItems: optionsMenu.contentData
    property alias actionVisible: action.visible
    property alias actionIconSource: action.iconSource
    property alias action2Visible: action2.visible
    property alias action2IconSource: action2.iconSource
    property alias action3Visible: action3.visible
    property alias action3IconSource: action3.iconSource

    signal actionPressed
    signal action2Pressed
    signal action3Pressed
    signal selected
    signal deselected

    signal imageError(var index)

    spacing: units.gu(1)

    state: "default"
    states: [
        State {
            name: "default"
            PropertyChanges { target: select; anchors.rightMargin: -select.width; opacity: anchors.rightMargin > -select.width ? 1 : 0; }
            PropertyChanges { target: menu; anchors.rightMargin: 0; opacity: 1 }
            //PropertyChanges { target: action; visible: actionIconSource !== "" }
        },
        State {
            name: "selection"
            PropertyChanges { target: select; anchors.rightMargin: 0 }
            PropertyChanges { target: menu; anchors.rightMargin: -select.width; opacity: anchors.rightMargin > -select.width ? 1 : 0; }
            //PropertyChanges { target: action; visible: false }
        }
    ]

    Rectangle {
        height: contentHeight
        width: image.visible ? coverSize : units.dp(1)
        color: "transparent"
        anchors {
            verticalCenter: parent.verticalCenter
            topMargin: units.gu(1)
            bottomMargin: units.gu(1)
        }

        Image {
            id: image
            anchors {
                verticalCenter: parent.verticalCenter
            }
            property int index: 0
            asynchronous: true
            fillMode: Image.PreserveAspectCrop
            height: coverSize
            width: height
            source: imageSources !== undefined && imageSources.length ? imageSources[index].art : noCover
            sourceSize.height: 512
            sourceSize.width: 512

            onStatusChanged: {
                if (status === Image.Error) {
                    row.imageError(index)
                    if (imageSources.length > (index + 1)) {
                        source = imageSources[++index].art
                    } else {
                        source = noCover
                    }
                } else if (status === Image.Ready) {
                    imageSource = source;
                }
            }
            visible: imageSources.length > 0
        }
    }

    Loader {
        id: columnComponent
        anchors {
            verticalCenter: parent.verticalCenter
        }

        width: imageSources === undefined ? parent.width - parent.spacing - action.width - menu.width
                                         : parent.width - image.width - parent.spacing - action.width - menu.width

        onSourceComponentChanged: {
            for (var i=0; i < item.children.length; i++) {
                if (item.children[i].text !== undefined) {
                    item.children[i].elide = Text.ElideRight
                    item.children[i].maximumLineCount = 1
                    item.children[i].wrapMode = Text.NoWrap
                    item.children[i].verticalAlignment = Text.AlignVCenter
                }
                // binds to width so it is updated when screen size changes
                item.children[i].width = Qt.binding(function () { return width; })
            }
        }
    }

    Item {
        id: favorite
        visible: isFavorite
        anchors.right: action2.left
        anchors.rightMargin: units.gu(1)
        width: visible ? units.gu(5) : 0

        Rectangle {
            color: "transparent"
            width: parent.width
            height: row.height

            Icon {
                width: parent.width
                height: width
                anchors.centerIn: parent
                source: "qrc:/images/starred.svg"
            }
        }
    }

    Item {
        id: action3
        visible: false
        anchors.right: action2.left
        anchors.rightMargin: units.gu(1)
        width: visible ? units.gu(5) : 0
        property alias iconSource: icon3.source

        Rectangle {
            color: "transparent"
            width: parent.width
            height: row.height

            Icon {
                id: icon3
                source: ""
                width: parent.width
                height: width
                anchors.centerIn: parent
                onClicked: action3Pressed()
            }
        }
    }

    Item {
        id: action2
        visible: false
        anchors.right: action.left
        anchors.rightMargin: units.gu(1)
        width: visible ? units.gu(5) : 0
        property alias iconSource: icon2.source

        Rectangle {
            color: "transparent"
            width: parent.width
            height: row.height

            Icon {
                id: icon2
                source: ""
                width: parent.width
                height: width
                anchors.centerIn: parent
                onClicked: action2Pressed()
            }
        }
    }

    Item {
        id: action
        visible: false
        anchors.right: menu.left
        anchors.rightMargin: units.gu(1)
        width: units.gu(5)
        property alias iconSource: icon.source

        Rectangle {
            color: "transparent"
            width: parent.width
            height: row.height

            Icon {
                id: icon
                source: ""
                width: parent.width
                height: width
                anchors.centerIn: parent
                onClicked: actionPressed()
            }
        }
    }

    Item {
        id: menu
        anchors.right: select.left
        width: visible ? units.gu(5) : 0
        visible: menuVisible

        Rectangle {
            color: "transparent"
            width: parent.width
            height: row.height

            Icon {
                width: menu.visible ? units.gu(5) : 0
                height: width
                anchors.centerIn: parent
                source: "qrc:/images/contextual-menu.svg"

                onClicked: optionsMenu.open()

                Menu {
                    id: optionsMenu
                    width: implicitWidth * units.scaleFactor
                    x: parent.width - width
                    transformOrigin: Menu.TopRight
                }
            }
        }

        Behavior on anchors.rightMargin {
            NumberAnimation { duration: 50 }
        }
    }

    Item {
        id: select
        anchors.right: parent.right
        width: units.gu(6)
        visible: true
        property alias checked: control.checked
        property alias checkable: control.checkable

        Rectangle {
            color: "transparent"
            width: control.width
            height: row.height

            MusicCheckBox {
                id: control
                anchors.centerIn: parent

                onClicked: {
                    if (checked) {
                        selected();
                    } else {
                        deselected();
                    }
                }
            }
        }

        Behavior on anchors.rightMargin {
            NumberAnimation { duration: 50 }
        }
    }

}
