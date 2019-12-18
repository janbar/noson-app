import QtQuick 2.2
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0

MouseArea {
    id: area
    property alias source: icon.source
    property alias label: label
    property color color: styleMusic.view.foregroundColor
    property color pressedColor: styleMusic.view.highlightedColor
    property alias rotationRunning: icon.rotationRunning
    property alias iconSize: icon.height
    height: units.gu(5)
    width: row.width + units.gu(2)
    enabled: true
    visible: true
    hoverEnabled: enabled && visible

    Row {
        id: row
        spacing: units.gu(0.5)
        height: parent.height
        visible: false
        anchors.verticalCenter: parent.verticalCenter

        Item {
            height: parent.height
            width: units.gu(0.5)
        }

        Image {
            id: icon
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height < units.gu(3) ? parent.height : (parent.height - units.gu(2))
            width: area.visible ? height : 0
            sourceSize.height: height
            sourceSize.width: width
            source: "qrc:/images/delete.svg"

            property bool rotationRunning: false

            onRotationRunningChanged: {
                if (rotationRunning)
                    animator.start();
                else {
                    animator.stop();
                    animator.angle = 0;
                }
            }

            Timer {
                id: animator
                interval: 500
                repeat: true
                property int angle: 0
                onTriggered: {
                    angle = (angle + 30) % 360;
                }
            }

            transform: Rotation {
                origin.x: icon.x - units.gu(1) + icon.width / 2
                origin.y: icon.y - units.gu(1) + icon.height / 2
                angle: animator.angle
            }
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
/*
IconButton {
    id: icn
    
    property string source: "qrc:/images/delete.svg"
    property alias color: label.color
    property alias label: label
    property int preferredWidth: (label.width > height ? label.width : height)

    height: units.gu(3)
    width: preferredWidth
    icon.source: source
    icon.width: height
    icon.height: height
    enabled: true
    visible: true
    
    Label {
        id: label
        anchors {
            top: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        font.pixelSize: units.fx("x-small")
    }
}
*/
