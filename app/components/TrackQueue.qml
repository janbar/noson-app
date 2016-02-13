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
import NosonApp 1.0

Item {
    property bool canLoad: false
    property bool completed: false

    property var model: QueueModel {
        Component.onCompleted: init(Sonos, "", false)
    }

    function loadQueue() {
        if (canLoad) {
            if (model.load()) {
                player.currentCount = model.count
                return completed = true
            }
            player.currentCount = 0
            return completed = false
        }
        return false
    }

    onCanLoadChanged: {
        mainView.currentlyWorking = true
        delayLoadQueue.start()
    }

    Timer {
        id: delayLoadQueue
        interval: 100
        onTriggered: {
            loadQueue()
            mainView.currentlyWorking = false
        }
    }

    Connections {
        target: model
        onDataUpdated: {
            mainView.currentlyWorking = true
            delayLoadQueue.start()
        }
    }
}
