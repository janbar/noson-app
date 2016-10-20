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
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/ListItemActions"
import "../components/HeadState"
import "../components/BottomEdge"

Page {
    id: groupPage

    property string pageTitle: i18n.tr("Group")
    property Item pageFlickable: groupList
    property string zoneId: ""

    header: PageHeader {
        flickable: groupPage.pageFlickable
        title: groupPage.pageTitle

        leadingActionBar {
            actions: [
                Action {
                    id: leaveGroup
                    iconName: "back"
                    objectName: "backAction"
                    onTriggered: {
                        mainView.currentlyWorking = true
                        delayHandleGroupChanges.start()
                    }
                }
            ]
            objectName: "groupLeadingActionBar"
        }
        trailingActionBar {
            actions: [
                Action {
                    iconName: "select"
                    text: i18n.tr("Select All")
                    visible: true

                    onTriggered: {
                        if (groupList.getSelectedIndices().length > 0) {
                            groupList.clearSelection()
                        } else {
                            groupList.selectAll()
                        }
                    }
                }
            ]
            objectName: "groupTrailingActionBar"
        }

        StyleHints {
            backgroundColor: mainView.headerColor
            dividerColor: Qt.darker(mainView.headerColor, 1.1)
        }
    }

    Timer {
        id: delayHandleGroupChanges
        interval: 100
        onTriggered: {
            var failure = false;
            var items = [];
            var indicies = groupList.getSelectedIndices();
            for (var i = 0; i < groupList.model.count; ++i) {
                var keep = false;
                for (var j = 0; j < indicies.length; j++) {
                    if (indicies[j] === i) {
                        keep = true;
                        break;
                    }
                }
                if (!keep) {
                    items.push(groupList.model.get(i));
                }
            }
            if (items.length == 0) {
                mainView.currentlyWorking = false;
            }
            else {
                for (var i = 0; i < items.length; ++i) {
                    if (!Sonos.unjoinRoom(items[i].payload)) {
                        failure = true;
                        break;
                    }
                }
                // Zones will be reloaded on signal topologyChanged
                // Signal is handled in MainView
                mainView.currentlyWorking = false
            }
            mainPageStack.pop()
        }
    }

    MultiSelectListView {
        id: groupList
        anchors {
            fill: parent
        }
        footer: Item {
            height: mainView.height - (styleMusic.common.expandHeight + groupList.currentHeight) + units.gu(8)
        }
        model: RoomsModel {
        }

        Component.onCompleted: {
            model.init(Sonos, false)
            model.load(zoneId)
            selectAll()
        }

        delegate: SelectListItem {
            id: groupListItem
            color: styleMusic.mainView.backgroundColor
            column: Column {
                Label {
                    id: roomName
                    color: model.coordinator ? UbuntuColors.blue : styleMusic.common.music
                    fontSize: "medium"
                    objectName: "nameLabel"
                    text: model.name
                }
            }
        }
    }
}
