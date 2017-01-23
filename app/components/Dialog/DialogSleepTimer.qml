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
import Ubuntu.Components.ListItems 1.3

DialogBase {
    id: dialogSleepTimer
    objectName: "dialogSleepTimer"
    // TRANSLATORS: this is a title of a dialog to configure standby timer
    title: i18n.tr("Standby timer")

    property int remainingTime: player.remainingSleepTimerDuration() // Load at startup

    Component.onCompleted: {
        selectorModel.insert(0, {name: i18n.tr("Disabled"),     duration: 0});
        selectorModel.insert(1, {name: i18n.tr("15 minutes"),   duration: 900});
        selectorModel.insert(2, {name: i18n.tr("30 minutes"),   duration: 1800});
        selectorModel.insert(3, {name: i18n.tr("45 minutes"),   duration: 2700});
        selectorModel.insert(4, {name: i18n.tr("1 hour"),       duration: 3600});
        selectorModel.insert(5, {name: i18n.tr("2 hours"),      duration: 7200});
        selectorModel.insert(6, {name: i18n.tr("3 hours"),      duration: 10800});
        selectorModel.insert(7, {name: i18n.tr("4 hours"),      duration: 14400});
        selectorModel.insert(8, {name: i18n.tr("5 hours"),      duration: 18000});
        selectorModel.insert(9, {name: i18n.tr("6 hours"),      duration: 21600});
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
        color: styleMusic.popover.labelColor
        elide: Text.ElideRight
        fontSize: "x-large"
        horizontalAlignment: Text.AlignHCenter
        opacity: 1.0
    }

    ListModel {
        id: selectorModel
    }

    ItemSelector {
        id: selector
        text: ""
        model: selectorModel
        containerHeight: itemHeight * 6
        selectedIndex: -1
        expanded: true
        multiSelection: false

        delegate: OptionSelectorDelegate {
            text: selected ? "<font color=\"white\">" + i18n.tr(model.name) + "</font>"
                           : "<font color=\"black\">" + model.name + "</font>"
        }

        onSelectedIndexChanged: {
            if (selectedIndex >= 0) {
                remainingTime = model.get(selectedIndex).duration;
            } else {
                selectedIndex: -1 // Reset selection
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
