import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    Label {
        id: label
        anchors.centerIn: parent
        text: qsTr("My Cover")
    }

    Image {
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectCrop
        source: "qrc:/images/noson.png"
        width: Math.min(parent.height, parent.width) * 0.80
        height: width
        sourceSize.height: 512
        sourceSize.width: 512
    }

    Image {
        id: backgroundImage
        anchors.centerIn: parent
        asynchronous: true
        fillMode: Image.PreserveAspectCrop
        height: parent.height
        source: player.currentMetaArt
        width: Math.max(parent.height, parent.width)
        sourceSize.height: 512
        sourceSize.width: 512
    }

    CoverActionList {
        id: coverAction

        CoverAction {
            iconSource: "image://theme/icon-cover-next"
            onTriggered: player.nextSong();
        }

        CoverAction {
            iconSource: player.isPlaying ? "image://theme/icon-cover-pause" : "image://theme/icon-cover-play"
            onTriggered: player.toggle();
        }
    }
}
