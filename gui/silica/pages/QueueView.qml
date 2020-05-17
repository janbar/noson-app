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
import NosonApp 1.0
import "../components"
import "../components/Dialog"


MusicPage {
    id: queuePage
    objectName: "queuePage"   
    pageTitle: qsTr("Queue")
    pageFlickable: queue
    pageMenuEnabled: true
    isListView: true
    listview: queue.listview

    Queue {
        id: queue
        anchors.fill: parent
    }

    Component {
        id: menuItemComp
        MenuItem {
        }
    }
    Component.onCompleted: {
        var newMenuItem = menuItemComp.createObject(pageMenuContent, {"text" : qsTr("Manage Queue") })
        newMenuItem.onClicked.connect(dialogManageQueue.open)
        newMenuItem = menuItemComp.createObject(pageMenuContent, {"text" : qsTr("Select Source") })
        newMenuItem.onClicked.connect(dialogSelectSource.open)
    }
}
