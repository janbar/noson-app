/*
 * Copyright (C) 2016
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

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3

DialogBase {
    id: dialogSleepTimer
    objectName: "dialogSleepTimer"
    // TRANSLATORS: this is a title of a dialog to configure standby timer
    title: i18n.tr("Standby timer")

    property int remainingTime: player.remainingSleepTimerDuration() // Load at startup

    Component.onCompleted: remainingTimer.start()

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
        color: styleMusic.popover.labelColor
        elide: Text.ElideRight
        fontSize: "x-large"
        horizontalAlignment: Text.AlignHCenter
        opacity: 1.0
    }

    Rectangle {
        id: timeSelector
        color: styleMusic.mainView.backgroundColor
        height: selector.containerHeight
        radius: units.gu(1.5)

        OptionSelector {
            id: selector
            model: [i18n.tr("Disabled"),
                    i18n.tr("15 minutes"),
                    i18n.tr("30 minutes"),
                    i18n.tr("45 minutes"),
                    i18n.tr("1 hour"),
                    i18n.tr("2 hours"),
                    i18n.tr("3 hours"),
                    i18n.tr("4 hours"),
                    i18n.tr("5 hours"),
                    i18n.tr("6 hours")]
            containerHeight: mainView.height > units.gu(60) ? itemHeight * 6 : itemHeight * 2
            selectedIndex: -1
            expanded: true
            multiSelection: false
            onSelectedIndexChanged: {
                if (selectedIndex >= 0) {
                    var sec = 0;
                    switch(selectedIndex)
                    {
                    case 9:
                        sec += 3600;
                    case 8:
                        sec += 3600;
                    case 7:
                        sec += 3600;
                    case 6:
                        sec += 3600;
                    case 5:
                        sec += 3600;
                    case 4:
                        sec += 900;
                    case 3:
                        sec += 900;
                    case 2:
                        sec += 900;
                    case 1:
                        sec += 900;
                    default:
                        break;
                    }
                    if (player.configureSleepTimer(sec))
                        remainingTime = sec
                    else
                        selectedIndex: -1 // Reset selection
                }
            }
        }
    }

    Button {
        text: i18n.tr("Close")
        color: styleMusic.dialog.cancelButtonColor
        onClicked: PopupUtils.close(dialogSleepTimer)
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
        return (hours<10 ? "0"+hours : hours) + ":" +
                (minutes<10 ? "0"+minutes : minutes) + ":" +
                (seconds<10 ? "0"+seconds : seconds);
    }
}
