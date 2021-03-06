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

DialogBase {
    id: dialog
    //: this is a title of a dialog to configure standby timer
    title: qsTr("Standby timer")

    property int remainingTime: 0

    cancelText: qsTr("Close")
    acceptText: ""
    canAccept: false

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
        font.pixelSize: 2 * units.fx("medium")
        horizontalAlignment: Text.AlignHCenter
        opacity: 1.0
    }

    ComboBox {
        id: selector
        menu: ContextMenu {
            MenuItem {text: qsTr("Disabled")}
            MenuItem {text: qsTr("15 minutes")}
            MenuItem {text: qsTr("30 minutes")}
            MenuItem {text: qsTr("45 minutes")}
            MenuItem {text: qsTr("1 hour")}
            MenuItem {text: qsTr("2 hours")}
            MenuItem {text: qsTr("3 hours")}
            MenuItem {text: qsTr("4 hours")}
            MenuItem {text: qsTr("5 hours")}
            MenuItem {text: qsTr("6 hours")}

            onActivated: {
                if (index >= 0) {
                    var sec = selectorModel.get(index).duration;
                    player.configureSleepTimer(sec, function(result) {
                        if (result) {
                            remainingTime = sec;
                        } else {
                            selector.currentIndex = -1; // Reset selection
                            mainView.actionFailed();
                        }
                    });
                }
            }
        }

        Layout.fillWidth: true
        currentIndex: 0
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
