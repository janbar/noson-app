/*
 * Copyright (C) 2018
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
        contentSpacing: units.gu(1)
        edgeMargins: units.gu(0)

        acceptText: qsTr("Save")
        cancelText: qsTr("Cancel")

        Row {
            width: parent.width
            spacing: units.gu(1)

            Item {
                id: roomLabel
                width: units.gu(5)
                height: room.height
                Icon {
                    anchors.centerIn: parent
                    height: units.gu(5)
                    width: height
                    color: styleMusic.dialog.labelColor
                    enabled: false
                    source: "qrc:/images/location.svg"
                    opacity: 0.7
                    hoverEnabled: false
                }
            }

            ComboBox {
                id: room
                textRole: "name"
                model: []
                width: parent.width - roomLabel.width - parent.spacing - units.gu(0.5)
                font.pixelSize: units.fx("medium")
                currentIndex: -1
                Component.onCompleted: {
                    popup.font.pointSize = font.pointSize;
                }
            }
        }

        Row {
            spacing: (parent.width - c1.width - c2.width - c3.width) / 2
            Column {
                id: c1
                Label {
                    width: parent.width
                    text: qsTr("Start time")
                    color: styleMusic.dialog.labelColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("medium")
                    horizontalAlignment: Text.AlignHCenter
                    opacity: 1.0
                }

                Row {
                    ClockTumbler {
                        id: startHour
                        model: 24
                        padding: 1
                        stepOnHold: 6
                    }
                    Label {
                        text: ":"
                        color: styleMusic.dialog.labelColor
                        font.pixelSize: units.fx("medium")
                        height: startHour.height
                        verticalAlignment: Text.AlignVCenter
                    }
                    ClockTumbler {
                        id: startMinute
                        model: 60
                        padding: 1
                        stepOnHold: 10
                    }
                }
            }
            Column {
                id: c2
                Label {
                    width: parent.width
                    text: qsTr("Duration")
                    color: styleMusic.dialog.labelColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("medium")
                    horizontalAlignment: Text.AlignHCenter
                    opacity: 1.0
                }

                Row {
                    ClockTumbler {
                        id: durationHours
                        model: [0,1,2]
                        padding: 1
                        stepOnHold: 1
                    }
                    Label {
                        text: ":"
                        color: styleMusic.dialog.labelColor
                        font.pixelSize: units.fx("medium")
                        height: durationHours.height
                        verticalAlignment: Text.AlignVCenter
                    }
                    ClockTumbler {
                        id: durationMinutes
                        model: 60
                        padding: 1
                        stepOnHold: 10
                    }
                }
            }

            Column {
                id: c3
                spacing: 0
                Item {
                    width: volume.width
                    height: units.gu(2)
                    Icon {
                        anchors.centerIn: parent
                        height: units.gu(5)
                        width: height
                        source: volume.value === 0 ? "qrc:/images/audio-volume-muted.svg" : "qrc:/images/audio-volume.svg"
                        enabled: false
                        opacity: 0.7
                        hoverEnabled: false
                    }
                }

                Slider {
                    id: volume
                    from: 0
                    value: 20
                    to: 100
                    orientation: Qt.Vertical
                    height: startHour.height
                    wheelEnabled: true
                }
            }
        }

        Row {
            Column {
                spacing: 0
                Label {
                    text: "  " + qsTr("Mon")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: monday
                    zoom: 0.66
                }
            }
            Column {
                spacing: 0
                Label {
                    text: "  " + qsTr("Tue")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: tuesday
                    zoom: 0.66
                }
            }
            Column {
                spacing: 0
                Label {
                    text: "  " + qsTr("Wed")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: wednesday
                    zoom: 0.66
                }
            }
            Column {
                spacing: 0
                Label {
                    text: "  " + qsTr("Thu")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: thursday
                    zoom: 0.66
                }
            }
            Column {
                spacing: 0
                Label {
                    text: "  " + qsTr("Fri")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: friday
                    zoom: 0.66
                }
            }
            Column {
                spacing: 0
                Label {
                    text: "  " + qsTr("Sat")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: saturday
                    zoom: 0.66
                }
            }
            Column {
                spacing: 0
                Label {
                    text: "  " + qsTr("Sun")
                    color: styleMusic.dialog.labelColor
                    font.pixelSize: units.fx("small")
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                MusicCheckBox {
                    id: sunday
                    zoom: 0.66
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

        Row {
            width: parent.width
            spacing: units.gu(1)

            Item {
                id: programLabel
                width: units.gu(5)
                height: program.height
                Icon {
                    anchors.centerIn: parent
                    height: units.gu(5)
                    width: height
                    color: styleMusic.dialog.labelColor
                    enabled: false
                    source: "qrc:/images/bell.svg"
                    opacity: 0.7
                    hoverEnabled: false
                }
            }

            ComboBox {
                id: program
                textRole: "title"
                model: programs
                width: parent.width - programLabel.width - parent.spacing - units.gu(0.5)
                font.pixelSize: units.fx("medium")
                currentIndex: 0
                Component.onCompleted: {
                    popup.font.pointSize = font.pointSize;
                }
            }
        }

        onOpened: {
            alarm.opened(); // forward signal
            var i = 0;
            room.currentIndex = -1;
            if (isNew) {
                var now = new Date();
                durationHours.currentIndex = 1;
                durationMinutes.currentIndex = 0;
                startHour.currentIndex = now.getHours();
                startMinute.currentIndex = now.getMinutes();
                if (roomModel != null && roomModel.count > 0) {
                    room.model = roomModel;
                    room.currentIndex = 0;
                    for (i = 0; i < roomModel.count; ++i)
                        if (roomModel.get(i).name === currentZone)
                            room.currentIndex = i
                } else
                    room.model = [];
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
                if (index > durationHours.model.count)
                    durationHours.currentIndex = durationHours.model.count - 1;
                else
                    durationHours.currentIndex = index;
                durationMinutes.currentIndex = model.duration.substr(3,2).valueOf();
                startHour.currentIndex = model.startLocalTime.substr(0,2).valueOf();
                startMinute.currentIndex = model.startLocalTime.substr(3,2).valueOf();
                if (roomModel != null && roomModel.count > 0) {
                    room.model = roomModel;
                    for (i = 0; i < roomModel.count; ++i)
                        if (roomModel.get(i).id === model.roomId)
                            room.currentIndex = i
                } else
                    room.model = [];
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

        onClosed: {
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
            model.startLocalTime = makeTime(startHour.currentIndex, startMinute.currentIndex, 0);
            model.duration = makeTime(durationHours.currentIndex, durationMinutes.currentIndex, 0);
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
