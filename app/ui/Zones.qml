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

BottomEdgePage {
    id: zonesPage
    title: i18n.tr("Zones")

    bottomEdgeEnabled: true
    bottomEdgePage: Page {
        title: currentZoneTag
        ZoneControls {
            controlledZone: currentZone
        }
    }
    bottomEdgeTitle: "" //i18n.tr("Zone controls")
    bottomEdgeMarging: musicToolbar.visible ? musicToolbar.height : 0

    state: zoneList.state === "multiselectable" ? "selection" : "default"
    states: [
        PageHeadState {
            id: defaultState
            name: "default"
            actions: [
                Action {
                    enabled: true
                    //iconName: "settings"
                    iconSource: Qt.resolvedUrl("../graphics/cogs.svg")
                    text: i18n.tr("Settings")
                    onTriggered: PopupUtils.open(Qt.resolvedUrl("../components/Dialog/DialogSettings.qml"), mainView)
                },
                Action {
                  enabled: true
                  iconName: "reload"
                  text: i18n.tr("Reload zones")
                  onTriggered: {
                      mainView.currentlyWorking = true
                      delayReloadZones.start()
                  }
                },
                Action {
                    enabled: zoneList.model.count > 1
                    //iconName: "compose"
                    iconSource: Qt.resolvedUrl("../graphics/group.svg")
                    text: i18n.tr("Create group")
                    onTriggered: zoneList.clearSelection() // change view state to multiselectable
                }
            ]
            PropertyChanges {
                target: zonesPage.head
                backAction: defaultState.backAction
                actions: defaultState.actions
            }
        },
        PageHeadState {
            id: selectionState
            name: "selection"
            actions: [
                Action {
                    enabled: zoneList.model.count > 0
                    iconName: zoneList.model.count > zoneList.getSelectedIndices().length ? "select" : "clear"
                    text: zoneList.model.count > zoneList.getSelectedIndices().length ? i18n.tr("Select All") : i18n.tr("Clear")

                    onTriggered: {
                        if (zoneList.model.count > zoneList.getSelectedIndices().length)
                            zoneList.selectAll()
                        else
                            zoneList.clearSelection()
                    }
                }
            ]
            backAction: Action {
                text: "back"
                iconName: "back"
                onTriggered: {
                    if (zoneList.getSelectedIndices().length > 1) {
                        mainView.currentlyWorking = true
                        delayJoinZones.start()
                    }
                    else {
                        zoneList.closeSelection()
                    }
                }
            }
            head: zonesPage.head

            PropertyChanges {
                target: zonesPage.head
                backAction: selectionState.backAction
                actions: selectionState.actions
            }
        }
    ]

    Timer {
        id: delayReloadZones
        interval: 100
        onTriggered: {
            reloadZone()
            mainView.currentlyWorking = false
        }
    }

    Timer {
        id: delayJoinZones
        interval: 100
        onTriggered: {
            handleJoinZones()
            zoneList.closeSelection()
            reloadZone()
            mainView.currentlyWorking = false
        }
    }

    function handleJoinZones() {
        var indicies = zoneList.getSelectedIndices();
        // get current as master
        for (var z = 0; z < zoneList.model.count; ++z) {
            if (zoneList.model.get(z).name === currentZone) {
                var master = zoneList.model.get(z)
                // join zones
                for (var i = 0; i < indicies.length; ++i) {
                    if (indicies[i] !== z) {
                        if (!Sonos.joinZone(zoneList.model.get(indicies[i]).payload, master.payload))
                            return false;
                    }
                }
                // all changes done
                return true;
            }
        }
        return false;
    }

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
                    fontSize: "medium"
                    objectName: "nameLabel"
                    text: model.isGroup ? model.shortName : model.name
                }

                Label {
                    id: fullName
                    color: styleMusic.common.subtitle
                    fontSize: "x-small"
                    objectName: "fillNameLabel"
                    text: model.name
                    visible: model.isGroup
                }
            }
            leadingActions: ListItemActions {
                actions: [
                    Clear {
                        enabled: model.isGroup
                        onTriggered: {
                            mainView.currentlyWorking = true
                            delayUnjoinZone.start()
                        }
                    }
                ]
            }
            multiselectable: true
            reorderable: false
            objectName: "zoneListItem" + index
            trailingActions: ListItemActions {
                actions: [
                    Action {
                        enabled: model.isGroup
                        iconName: "edit-cut"
                        text: i18n.tr("Group")

                        onTriggered: {
                            mainPageStack.push(Qt.resolvedUrl("Group.qml"),
                                               {"zoneId": model.id})
                        }
                    }
                ]
                delegate: ActionDelegate {

                }
            }

            onItemClicked: {
                mainView.currentlyWorking = true
                delayChangeZone.start()
            }

            Timer {
                id: delayChangeZone
                interval: 100
                onTriggered: {
                    if (currentZone !== model.name) {
                        customdebug("Connecting zone '" + name + "'");
                        if ((Sonos.connectZone(model.name) || Sonos.connectZone("")) && player.connect()) {
                            currentZone = Sonos.getZoneName();
                            currentZoneTag = Sonos.getZoneShortName();
                            if (noZone)
                                noZone = false;
                        }
                        else {
                            if (!noZone)
                                noZone = true;
                        }
                    }
                    mainView.currentlyWorking = false
                }
            }

            Timer {
                id: delayUnjoinZone
                interval: 100
                onTriggered: {
                    Sonos.unjoinZone(model.payload)
                    reloadZone()
                    mainView.currentlyWorking = false
                }
            }

            onSelectedChanged: {
                if (zoneList.state === "multiselectable")
                    zoneList.checkSelected()
            }

        }

        onStateChanged: {
            if (state === "multiselectable")
                selectCurrentZone()
        }

        function selectCurrentZone() {
            var tmp = [];
            for (var i = 0; i < model.count; i++) {
                if (model.get(i).name === currentZone) {
                    tmp.push(i);
                    break;
                }
            }
            ViewItems.selectedIndices = tmp
        }

        function checkSelected() {
            // keep currentZone selected
            var indicies = getSelectedIndices();
            for (var i = 0; i < indicies.length; i++) {
                if (model.get(indicies[i]).name === currentZone)
                    return;
            }
            for (var i = 0; i < model.count; i++) {
                if (model.get(i).name === currentZone) {
                    indicies.push(i);
                    ViewItems.selectedIndices = indicies;
                    break;
                }
            }
        }

    }
}
