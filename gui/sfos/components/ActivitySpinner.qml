import QtQuick 2.2
import Sailfish.Silica 1.0

Item {
    id: loading
    height: 96
    width: parent.width
    visible: false

    anchors {
        horizontalCenter: parent.horizontalCenter
        verticalCenter: parent.verticalCenter
    }

    BusyIndicator {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        running: loading.visible
        height: parent.height
        width: height
        opacity: 0.9
        z: 1
    }
}
