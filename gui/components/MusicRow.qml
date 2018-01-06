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
    property real coverSize: styleMusic.common.albumSize
    property string noCover: "qrc:/images/no_cover.png"
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
    signal actionPressed
    signal action2Pressed
    signal selected
    signal deselected

    signal imageError

    spacing: units.gu(2)

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

    Image {
        id: image
        anchors {
            verticalCenter: parent.verticalCenter
            topMargin: units.gu(1)
            bottomMargin: units.gu(1)
        }
        asynchronous: true
        fillMode: Image.PreserveAspectCrop
        height: contentHeight
        width: height
        source: imageSource !== "" ? imageSource : noCover
        sourceSize.height: height
        sourceSize.width: width

        onStatusChanged: {
            if (status === Image.Error) {
                row.imageError()
                source = noCover
            }
        }
        visible: imageSource !== ""
    }

    Rectangle {
        height: contentHeight
        width: units.dp(1)
        color: "transparent"
        visible: !image.visible
    }

    Loader {
        id: columnComponent
        anchors {
            verticalCenter: parent.verticalCenter
        }

        width: imageSource === undefined ? parent.width - parent.spacing - action.width - menu.width
                                         : parent.width - image.width - parent.spacing - action.width - menu.width

        onSourceComponentChanged: {
            for (var i=0; i < item.children.length; i++) {
                item.children[i].elide = Text.ElideRight
                item.children[i].maximumLineCount = 1
                item.children[i].wrapMode = Text.NoWrap
                item.children[i].verticalAlignment = Text.AlignVCenter

                // binds to width so it is updated when screen size changes
                item.children[i].width = Qt.binding(function () { return width; })
            }
        }
    }

    Item {
        id: favorite
        anchors.right: action2.left
        anchors.rightMargin: units.gu(2)
        width: units.gu(3)

        Rectangle {
            color: "transparent"
            width: parent.width
            height: row.height

            Image {
                width: parent.width
                height: width
                anchors.centerIn: parent
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter
                source: isFavorite ? "qrc:/images/starred.svg" : ""
            }
        }
    }

    Item {
        id: action2
        visible: false
        anchors.right: action.left
        anchors.rightMargin: units.gu(2)
        width: visible ? units.gu(3) : 0
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
        anchors.rightMargin: units.gu(2)
        width: units.gu(3)
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
                width: menu.visible ? units.gu(3) : 0
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
