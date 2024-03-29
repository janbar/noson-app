/*
 * Copyright (C) 2017
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
import QtQuick.Controls 2.2
import NosonApp 1.0
import "components"
import "components/Dialog"
import "../toolbox.js" as ToolBox

MusicPage {
    id: queuePage
    pageTitle: qsTr("Queue")
    isListView: true
    listview: queue.listview

    QueueModel {
        id: model
        onDataUpdated: asyncLoad()
        onLoaded: resetModel()
    }

    Connections {
        target: player
        onConnectedChanged: {
            if (player.connected) {
                model.init(player.zoneHandle(), false);
                // focus current index
                if (player.currentIndex > 0) {
                    model.fetchAt(player.currentIndex - 1);
                    queue.saveViewFocus(player.currentIndex, ListView.Beginning);
                } else {
                    model.asyncLoad();
                }
            }
        }
    }

    Component.onCompleted: {
        if (player.connected) {
            model.init(player.zoneHandle(), false);
            // focus current index
            if (player.currentIndex > 0) {
                model.fetchAt(player.currentIndex - 1);
                queue.saveViewFocus(player.currentIndex, ListView.Beginning);
            } else {
                model.asyncLoad();
            }
        }
    }

    Queue {
        id: queue
        anchors.fill: parent
        queueModel: model
    }

    // Page actions
    optionsMenuVisible: true
    optionsMenuContentItems: [
        MenuItem {
            visible: (queue.listview.count > 0)
            height: (visible ? implicitHeight : 0)
            text: qsTr("Manage queue")
            font.pointSize: units.fs("medium")
            onTriggered: dialogManageQueue.open()
        },
        MenuItem {
            text: qsTr("Select source")
            font.pointSize: units.fs("medium")
            onTriggered: dialogSelectSource.open()
        }
    ]

    Connections {
        target: mainView
        onWideAspectChanged: {
            if (wideAspect) {
                stackView.pop();
                // previous page can be over queuePage, i.e AddToPlaylist
                if (stackView.currentItem === queuePage) {
                    stackView.pop();
                }
            }
        }
    }
}
