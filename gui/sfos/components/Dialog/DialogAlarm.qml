/*
 * Copyright (C) 2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Adam Pigg <adam@piggz.co.uk>
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
import QtQuick.Layouts 1.1
import NosonApp 1.0
import "../"

Item {
    id: alarm

    property var container: null
    property var roomModel: null

    signal opened
    signal closed

    DialogBase {
        id: dialog
        property var model: null
        property bool isNew: false;
        property int newIndex: -1

        //: this is a title of a dialog to configure an alarm
        title: qsTr("Alarm")
        contentSpacing: units.gu(2)

        acceptText: qsTr("Save")
        cancelText: qsTr("Cancel")

        Row {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
            spacing: Theme.paddingSmall

            ComboBox {
                id: room
                label: qsTr("Room")
                
                menu: ContextMenu {
                    Repeater {
                        id: roomRepeater
                        //model: roomModel
                        delegate: MenuItem {
                            text: name
                        }
                    }
                }
                width: parent.width
                currentIndex: -1
            }
        }

        property real pickerSize: (width > height ? height / 3.0 : width / 2) - Theme.paddingSmall

        RowLayout {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: 0
                rightMargin: 0
            }
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            spacing: Theme.paddingSmall
            Column {
                id: c1
                width: dialog.pickerSize
                Label {
                    width: parent.width
                    text: qsTr("Start time")
                    color: styleMusic.dialog.labelColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("medium")
                    horizontalAlignment: Text.AlignHCenter
                }

                TimePicker {
                    id: tpStart
                    hourMode: DateTime.TwentyFourHours
                    width: parent.width
                    Text {
                            anchors.centerIn: parent
                            horizontalAlignment: Text.AlignHCenter
                            color: styleMusic.dialog.labelColor
                            font.pixelSize: units.fx("medium")
                            width: parent.width
                            text: tpStart.timeText
                    }
                }
            }
            Column {
                id: c2
                width: dialog.pickerSize
                Label {
                    width: parent.width
                    text: qsTr("Duration")
                    color: styleMusic.dialog.labelColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("medium")
                    horizontalAlignment: Text.AlignHCenter
                }

                TimePicker {
                    id: tpDuration
                    hourMode: DateTime.TwentyFourHours
                    width: parent.width
                    Text {
                            anchors.centerIn: parent
                            horizontalAlignment: Text.AlignHCenter
                            color: styleMusic.dialog.labelColor
                            font.pixelSize: units.fx("medium")
                            width: parent.width
                            text: tpDuration.timeText
                    }
                }
            }
        }
        
        RowLayout {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Column {
                spacing: 0
                Label {
                    text: qsTr("Mon")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: monday
                    zoom: 0.80
                }
            }
            Column {
                spacing: 0
                Label {
                    text: qsTr("Tue")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: tuesday
                    zoom: 0.80
                }
            }
            Column {
                spacing: 0
                Label {
                    text: qsTr("Wed")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: wednesday
                    zoom: 0.80
                }
            }
            Column {
                spacing: 0
                Label {
                    text: qsTr("Thu")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: thursday
                    zoom: 0.80
                }
            }
            Column {
                spacing: 0
                Label {
                    text: qsTr("Fri")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: friday
                    zoom: 0.80
                }
            }
            Column {
                spacing: 0
                Label {
                    text: qsTr("Sat")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: saturday
                    zoom: 0.80
                }
            }
            Column {
                spacing: 0
                Label {
                    text: qsTr("Sun")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: sunday
                    zoom: 0.80
                }
            }
        }

        ListModel {
            id: programs
            property var metadata: []
            function reset() {
                clear();
                metadata = [];
            }
        }
        
        ListModel {
            id: rooms
        }
            
        Row {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
            spacing: Theme.paddingSmall

            Item {
                id: programIcon
                width: units.gu(5)
                height: program.height
                Icon {
                    anchors.centerIn: parent
                    height: units.gu(5)
                    width: height
                    color: styleMusic.dialog.labelColor
                    enabled: false
                    source: "image://theme/icon-m-alarm"
                    opacity: 0.7
                }
            }

            ComboBox {
                id: program
                menu: ContextMenu {
                    Repeater {
                        model: programs
                        delegate: MenuItem {
                            text: title
                        }
                    }
                }
                width: parent.width - programIcon.width - parent.spacing
                currentIndex: 0
            }
        }

        Row {
            id: c3
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
            spacing: Theme.paddingSmall

            Icon {
                id: volumeIcon
                height: units.gu(5)
                width: height
                source: volume.value === 0 ? "qrc:/sfos/images/icon-m-speaker-muted.svg" : "qrc:/sfos/images/icon-m-speaker.svg"
                enabled: false
                opacity: 0.7
            }

            StyledSlider {
                id: volume
                minimumValue: 0
                value: 20
                maximumValue: 100
                width: parent.width - volumeIcon.width - 2 * parent.spacing
            }
        }

        onOpened: {
            alarm.opened(); // forward signal
            var i = 0;
            room.currentIndex = -1;
            if (isNew) {
                var now = new Date();
                tpDuration.hour = 1;
                tpDuration.minute = 0;
                tpStart.hour = now.getHours();
                tpStart.minute = now.getMinutes();
                if (roomModel != null && roomModel.count > 0) {
                    roomRepeater.model = roomModel;
                    room.currentIndex = 0;
                    for (i = 0; i < roomModel.count; ++i)
                        if (roomModel.get(i).name === currentZone)
                            room.currentIndex = i
                } else {
                    roomRepeater.model = [];
                }
                volume.value = model.volume;
                monday.checked = true;
                tuesday.checked = true;
                wednesday.checked = true;
                thursday.checked = true;
                friday.checked = true;
                saturday.checked = false;
                sunday.checked = false;
            } else {
                var index = model.duration.substr(0,2).valueOf();                
                if (index > 23) {
                    tpDuration.hour = 23;
                } else {
                    tpDuration.hour = parseInt(index);
                }
                tpDuration.minute = model.duration.substr(3,2).valueOf();
                tpStart.hour = model.startLocalTime.substr(0,2).valueOf();
                tpStart.minute = model.startLocalTime.substr(3,2).valueOf();
                if (roomModel != null && roomModel.count > 0) {
                    roomRepeater.model = roomModel;
                    for (i = 0; i < roomModel.count; ++i)
                        if (roomModel.get(i).id === model.roomId)
                            room.currentIndex = i
                } else {
                    roomRepeater.model = [];
                }
                volume.value = model.volume;
                monday.checked = model.recurrence.indexOf("MON") >= 0;
                tuesday.checked = model.recurrence.indexOf("TUE") >= 0;
                wednesday.checked = model.recurrence.indexOf("WED") >= 0;
                thursday.checked = model.recurrence.indexOf("THU") >= 0;
                friday.checked = model.recurrence.indexOf("FRI") >= 0;
                saturday.checked = model.recurrence.indexOf("SAT") >= 0;
                sunday.checked = model.recurrence.indexOf("SUN") >= 0;
            }
            loadPrograms();
        }

        onDone: {
            alarm.closed(); // forward signal
        }

        function loadPrograms() {
            var i = 0;
            programs.reset();
            programs.insert(i, { uri: "x-rincon-buzzer:0", title: "BUZZER" });
            programs.metadata.push(null);
            program.currentIndex = 0;
            if (model.programUri !== programs.get(0).uri) {
                programs.insert(++i, { uri: model.programUri, title: model.programTitle });
                programs.metadata.push(model.programMetadata);
                program.currentIndex = i;
            }
            for (var f = 0; f < AllFavoritesModel.count; ++f) {
                var favorite = AllFavoritesModel.get(f);
                programs.insert(++i, { uri: favorite.objectUri, title: favorite.title });
                programs.metadata.push(favorite.object);
            }
        }

        function makeTime(hours, minutes, seconds) {
            return (hours < 10 ? "0" + hours : hours) + ":" +
                    (minutes < 10 ? "0" + minutes : minutes) + ":" +
                    (seconds < 10 ? "0" + seconds : seconds);
        }

        function makeRecurrence() {
            var str = "";
            if (monday.checked)
                str = str + "MON,";
            if (tuesday.checked)
                str = str + "TUE,";
            if (wednesday.checked)
                str = str + "WED,";
            if (thursday.checked)
                str = str + "THU,";
            if (friday.checked)
                str = str + "FRI,";
            if (saturday.checked)
                str = str + "SAT,";
            if (sunday.checked)
                str = str + "SUN,";
            if (str.length > 0)
                return str.substr(0, str.length - 1);
            return str;
        }

        onAccepted: {
            model.roomId = roomModel.get(room.currentIndex).id;
            model.volume = volume.value;
            model.recurrence = makeRecurrence();
            model.startLocalTime = makeTime(tpStart.hour, tpStart.minute, 0);
            model.duration = makeTime(tpDuration.hour, tpDuration.minute, 0);
            model.programUri = programs.get(program.currentIndex).uri;
            model.programMetadata = programs.metadata[program.currentIndex];
            if (isNew) {
                if (!Sonos.createAlarm(model.payload))
                    popInfo.open(qsTr("Action can't be performed"));
                    alarm.container.remove(newIndex);
            } else {
                if (!Sonos.updateAlarm(model.payload)) {
                    popInfo.open(qsTr("Action can't be performed"));
                    alarm.container.asyncLoad();
                }
            }
        }

        onRejected: {
            if (isNew)
                alarm.container.remove(newIndex);
        }
    }

    function open(model, isNew, newIndex) {
        alarm.container = container;
        dialog.model = model;
        if (isNew !== undefined && isNew) {
            dialog.isNew = isNew;
            dialog.newIndex = newIndex;
        } else {
            dialog.isNew = false;
            dialog.newIndex = -1;
        }
        return dialog.open();
    }
}
