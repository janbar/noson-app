/*
 * Copyright (C) 2016, 2017
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

import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

DialogBase {
    id: dialog
    //: this is a title of a dialog to configure standby timer
    title: qsTr("Standby timer")

    property int remainingTime: 0

    footer: Row {
        leftPadding: units.gu(1)
        rightPadding: units.gu(1)
        bottomPadding: units.gu(1)
        spacing: units.gu(1)
        layoutDirection: Qt.RightToLeft

        Button {
            flat: true
            text: qsTr("Close")
            onClicked: dialog.reject()
        }
    }

    onOpened: {
        player.remainingSleepTimerDuration(function(result) {
            remainingTime = result;
            for (var i = 0; i < selectorModel.count; ++i) {
                if (remainingTime <= selectorModel.get(i).duration) {
                    selector.currentIndex = i;
                    break;
                }
            }
        });
    }

    ListModel {
        id: selectorModel
        ListElement {text: qsTr("Disabled");     duration: 0}
        ListElement {text: qsTr("15 minutes");   duration: 900}
        ListElement {text: qsTr("30 minutes");   duration: 1800}
        ListElement {text: qsTr("45 minutes");   duration: 2700}
        ListElement {text: qsTr("1 hour");       duration: 3600}
        ListElement {text: qsTr("2 hours");      duration: 7200}
        ListElement {text: qsTr("3 hours");      duration: 10800}
        ListElement {text: qsTr("4 hours");      duration: 14400}
        ListElement {text: qsTr("5 hours");      duration: 18000}
        ListElement {text: qsTr("6 hours");      duration: 21600}
    }

    Component.onCompleted: {
        // launch timer animation
        remainingTimer.start()
    }

    Timer {
        id: remainingTimer
        interval: 1000
        repeat: true
        onTriggered: {
            if (remainingTime > 0) {
                --remainingTime
            }
            remainingLabel.text = remainingTimeToString(remainingTime)
        }
    }

    Label {
        id: remainingLabel
        anchors {
            left: parent.left
            leftMargin: units.gu(1)
            right: parent.right
            rightMargin: units.gu(1)
        }
        text: remainingTimeToString(remainingTime)
        color: styleMusic.dialog.labelColor
        elide: Text.ElideRight
        font.pointSize: 2 * units.fs("medium")
        horizontalAlignment: Text.AlignHCenter
        opacity: 1.0
    }

    ComboBox {
        id: selector
        textRole: "text"
        model: selectorModel
        Layout.fillWidth: true
        font.pointSize: units.fs("medium")
        currentIndex: 0
        Component.onCompleted: {
            popup.font.pointSize = font.pointSize;
        }
        onActivated: {
            if (index >= 0) {
                var sec = model.get(index).duration;
                player.configureSleepTimer(sec, function(result) {
                    if (result) {
                        remainingTime = sec;
                    } else {
                        currentIndex = -1; // Reset selection
                        mainView.actionFailed();
                    }
                });
            }
        }
    }

    function remainingTimeToString(remaining) {
        var hours = Math.floor(remaining / 3600);
        var minutes = Math.floor((remaining % 3600) / 60);
        var seconds = remaining % 60;
        // Make sure that we never see "NaN:NaN:NaN"
        if (hours.toString() == 'NaN')
            hours = 0;
        if (minutes.toString() == 'NaN')
            minutes = 0;
        if (seconds.toString() == 'NaN')
            seconds = 0;
        return (hours < 10 ? "0" + hours : hours) + ":" +
                (minutes < 10 ? "0" + minutes : minutes) + ":" +
                (seconds < 10 ? "0" + seconds : seconds);
    }
}
