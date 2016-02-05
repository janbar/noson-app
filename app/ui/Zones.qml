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
import Ubuntu.Components 1.2
import Ubuntu.Components.Popups 1.0
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/ListItemActions"
import "../components/HeadState"
import "../components/BottomEdge"

BottomEdgePage {
    id: zonesPage
    title: i18n.tr("Zones")

    bottomEdgeEnabled: true
    bottomEdgePage: Page {
        title: currentZone
        ZoneControls {
            controlledZone: currentZone
        }
    }
    bottomEdgeTitle: i18n.tr("Zone controls")
    bottomEdgeMarging: musicToolbar.visible ? musicToolbar.height : 0

    state: zoneList.state === "multiselectable" ? "selection" : "default"
    states: [
        PageHeadState {
            id: defaultState
            name: "default"
            actions: [
                Action {
                    enabled: true
                    iconName: "settings"
                    text: i18n.tr("Settings")
                    onTriggered: PopupUtils.open(Qt.resolvedUrl("../components/Dialog/DialogSettings.qml"), mainView)
                },
                Action {
                  enabled: true
                  iconName: "reload"
                  text: i18n.tr("Reload zones")
                  onTriggered: connectZone()
                },
                Action {
                    enabled: true
                    iconName: "close"
                    text: i18n.tr("Quit")
                    onTriggered: Qt.quit()
                }

            ]
            PropertyChanges {
                target: zonesPage.head
                backAction: defaultState.backAction
                actions: defaultState.actions
            }
        },
        MultiSelectHeadState {
            id: selectionState
            name: "selection"
            removable: false
            addToQueue: false
            addToPlaylist: false
            listview: zoneList
            thisPage: zonesPage
        }
    ]

    MultiSelectListView {
        id: zoneList
        anchors {
            fill: parent
        }
        footer: Item {
            height: mainView.height - (styleMusic.common.expandHeight + zoneList.currentHeight) + units.gu(8)
        }
        model: AllZonesModel
        objectName: "zoneList"

        delegate: MusicListItem {
            id: zoneListItem
            color: currentZone === model.name ? "#2c2c34" : styleMusic.mainView.backgroundColor
            column: Column {
                Label {
                    id: zoneName
                    color: currentZone === model.name ? UbuntuColors.blue : styleMusic.common.music
                    fontSize: "large"
                    objectName: "nameLabel"
                    text: model.name
                }
            }
            leadingActions: ListItemActions {
                actions: [
                    Clear {
                        onTriggered: {}
                    }
                ]
            }
            multiselectable: true
            objectName: "zoneListItem" + index
            reorderable: false
            trailingActions: ListItemActions {
                actions: []
                delegate: ActionDelegate {

                }
            }

            onItemClicked: {
                if (currentZone !== model.name) {
                    customdebug("Connecting zone '" + name + "'");
                    if ((Sonos.connectZone(model.name) || Sonos.connectZone("")) && player.connect()) {
                        currentZone = Sonos.getZoneName();
                        if (noZone)
                            noZone = false;
                    }
                    else {
                        if (!noZone)
                            noZone = true;
                    }
                }
            }
        }
    }
}
