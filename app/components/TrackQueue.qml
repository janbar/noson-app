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
        if (canLoad)
            model.asyncLoad()
    }

    onCanLoadChanged: {
        if (canLoad)
            model.asyncLoad()
    }

    Connections {
        target: model
        onDataUpdated: {
            if (canLoad)
                model.asyncLoad()
        }
        onLoaded: {
            if (succeeded) {
                model.resetModel()
                player.currentCount = model.count
                completed = true
            } else {
                model.resetModel()
                player.currentCount = 0
                completed = false
            }
        }
    }
}
