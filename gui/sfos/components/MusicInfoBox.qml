/*
 * Copyright (C) 2019
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

import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: infoBox

    Row {
        id: row
        spacing: units.gu(1)
        anchors.verticalCenter: parent.verticalCenter

        MusicIcon {
            id: infoSync
            color: styleMusic.view.foregroundColor
            width: Theme.iconSizeSmall
            height: width
            source: "image://theme/icon-s-device-download"
            visible: shareIndexInProgress
            animationRunning: shareIndexInProgress
        }

        MusicIcon {
            id: infoTimer
            color: styleMusic.view.foregroundColor
            width: Theme.iconSizeSmall
            height: width
            source: "image://theme/icon-s-timer"
            visible: player.sleepTimerEnabled
        }

        MusicIcon {
            id: infoAlarm
            color: styleMusic.view.foregroundColor
            width: Theme.iconSizeSmall
            height: width
            source: "image://theme/icon-s-alarm"
            visible: alarmEnabled
        }
    }
}
