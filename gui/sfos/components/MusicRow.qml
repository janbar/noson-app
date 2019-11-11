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

import QtQuick 2.2
import Sailfish.Silica 1.0
import "ListItemActions"

Item {
    id: row
    
    property real contentHeight: units.gu(6)
    property alias column: columnComponent.sourceComponent
    property real coverSize: styleMusic.view.albumSize
    property string noCover: "qrc:/images/no_cover.png"
    property var imageSources: []
    property string imageSource: ""
    property string description: ""
    property bool isFavorite: false
    property alias checked: control.checked

    property bool menuVisible: false
    property alias actionVisible: action.visible
    property string actionIconSource: ""
    property alias action2Visible: action2.visible
    property string action2IconSource: ""
    signal actionPressed
    signal action2Pressed
    signal selected
    signal deselected

    signal imageError(var index)
            
    anchors.leftMargin: units.gu(1)
    anchors.rightMargin: units.gu(2)

    height: contentHeight

    state: "default"
    states: [
        State {
            name: "default"
            PropertyChanges { target: select; anchors.rightMargin: -select.width; opacity: anchors.rightMargin > -select.width ? 1 : 0; }
            //PropertyChanges { target: action; visible: actionIconSource !== "" }
        },
        State {
            name: "selection"
            PropertyChanges { target: select; anchors.rightMargin: 0 }
            //PropertyChanges { target: action; visible: false }
        }
    ]

    Image {
        id: image
        property int index: 0
        anchors {
            //verticalCenter: parent.verticalCenter
            topMargin: units.gu(1)
            bottomMargin: units.gu(1)
        }
        asynchronous: true
        fillMode: Image.PreserveAspectCrop
        height: contentHeight
        width: height
        source: imageSources !== undefined && imageSources.length ? imageSources[index].art : row.noCover
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
            left: image.right
            leftMargin: units.gu(2)
        }

        width: imageSources === undefined ? parent.width - parent.spacing - action.width
                                        : parent.width - image.width - parent.spacing - action.width

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
        id: action2
        visible: false
        anchors.right: action.left
        anchors.rightMargin: units.gu(1)
        width: visible ? units.gu(5) : 0

        Rectangle {
            color: "transparent"
            width: parent.width
            height: row.height

            NosonIcon {
                id: icon2
                source: row.action2IconSource
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
        anchors.right: select.left
        anchors.rightMargin: units.gu(1)
        width: units.gu(5)

        Rectangle {
            color: "transparent"
            width: parent.width
            height: row.height

            NosonIcon {
                id: icn
                source: row.actionIconSource
                width: parent.width
                height: width
                anchors.centerIn: parent
                onClicked: actionPressed()
            }
        }
    }

    Item {
        id: select
        anchors.right: parent.right
        width: units.gu(6)
        visible: true
        property alias checked: control.checked

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

