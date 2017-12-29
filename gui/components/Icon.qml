import QtQuick 2.9
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0

MouseArea {
    id: area
    property alias source: icon.source
    property alias label: label
    property color color: "dimgray"
    property color pressedColor: "lightgray"
    height: units.gu(4)
    width: row.width
    enabled: true
    visible: true

    Row {
        id: row
        spacing: units.gu(0.5)
        height: parent.height
        visible: false

        Image {
            id: icon
            height: parent.height
            width: area.visible ? height : 0
            source: "qrc:/images/delete.svg"
        }

        Label {
            id: label
            anchors.verticalCenter: parent.verticalCenter
            width: area.visible ? implicitWidth : 0
        }
    }

    Rectangle {
        id: iconFill
        visible: false
        anchors.fill: row
        color: parent.pressed ? parent.pressedColor : parent.color
    }

    OpacityMask {
        anchors.fill: iconFill
        source: iconFill
        maskSource: row
    }

}
