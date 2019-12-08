import QtQuick 2.2
import Sailfish.Silica 1.0

IconButton {
    id: icn
    
    property string source: "qrc:/images/delete.svg"
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
    }
}
