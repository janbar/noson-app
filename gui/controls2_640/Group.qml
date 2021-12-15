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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQml.Models 2.3
import NosonApp 1.0
import "components"
import "components/Delegates"
import "components/Flickables"

MusicPage {
    id: groupPage
    objectName: "groupPage"
    pageTitle: qsTr("Group")
    pageFlickable: groupList
    isListView: true
    listview: groupList

    state: "group"

    property string zoneId: ""

    onGroupNoneRoomClicked: {
        groupList.selectNone()
    }

    onGroupRoomClicked: {
        if (handleUnjoinRooms())
            stackView.pop();
    }

    function handleUnjoinRooms() {
        // keep back unselected rooms
        var rooms = [];
        var indicies = groupList.getSelectedIndices();
        var keep = false;
        for (var i = 0; i < roomsModel.count; ++i) {
            keep = false;
            for (var j = 0; j < indicies.length; j++) {
                if (indicies[j] === i) {
                    keep = true;
                    break;
                }
            }
            if (!keep) {
                rooms.push(roomsModel.get(i).payload);
            }
        }
        // start unjoin rooms
        if (rooms.length > 0) {
            var future = Sonos.tryUnjoinRooms(rooms);
            future.finished.connect(mainView.actionFinished);
            return future.start();
        }
        return true;
    }

    RoomsModel {
        id: roomsModel
    }

    BlurredBackground {
        id: blurredBackground
        height: parent.height
    }

    MultiSelectListView {
        id: groupList
        anchors.fill: parent

        state: "selection"

        model: DelegateModel {
            id: visualModel
            model: roomsModel

            delegate: SelectMusicListItem {
                id: listItem
                listview: groupList
                reorderable: false
                selectable: true
                highlighted: false

                color: "transparent"
                description: qsTr("Room")
                actionVisible: false
                action2Visible: false
                menuVisible: false

                column: Column {
                    Label {
                        id: roomName
                        color: model.coordinator ? "#19b1e9" : styleMusic.view.foregroundColor
                        font.pointSize: units.fs("large")
                        objectName: "nameLabel"
                        text: model.name
                    }
                }
            }
        }

        Component.onCompleted: {
            roomsModel.load(Sonos, zoneId);
            selectAll();
        }
    }
}
