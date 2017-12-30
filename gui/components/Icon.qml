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
    hoverEnabled: enabled && visible

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

    Rectangle {
        id: ripple
        readonly property bool square: area.implicitWidth <= area.implicitHeight
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        clip: !square
        width: parent.width
        height: parent.height
        radius: height / 2
        color: area.color
        opacity: 0

        Behavior on opacity {
            NumberAnimation { duration: 100 }
        }
    }

    onEntered: ripple.opacity = 0.1
    onExited: ripple.opacity = 0
}
