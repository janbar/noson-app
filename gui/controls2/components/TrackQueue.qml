/*
 * Copyright (C) 2016-2019
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

import QtQuick 2.9
import NosonApp 1.0

Item {
    property int trackCount: 0
    property alias model: queue

    QueueModel {
        id: queue
    }

    // Initialize the queue for given zone player
    function initQueue(zonePlayer) {
        if (zonePlayer) {
            queue.init(zonePlayer, "", false);
            queue.asyncLoad();
        }
    }

    function loadQueue() {
        queue.asyncLoad()
    }

    Connections {
        target: queue
        function onDataUpdated() {
            queue.asyncLoad();
        }
        function onLoaded(succeeded) {
            if (succeeded) {
                queue.resetModel()
                trackCount = queue.count
            } else {
                queue.resetModel()
                trackCount = 0
            }
        }
    }
}
