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
import QtQml.Models 2.3
import NosonApp 1.0
import NosonThumbnailer 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"

MusicPage {
    id: zonesPage
    objectName: "zonePage"
    pageTitle: qsTr("Zones")
    pageFlickable: zoneList
    pageMenuEnabled: false
    isListView: true
    listview: zoneList

    state: "zone"

    onReloadClicked: {
        Thumbnailer.reset(); // reset thumbnailer state
        connectSonos();
    }

    onGroupAllZoneClicked: {
        zoneList.selectAll()
    }

    onGroupZoneClicked: {
        handleJoinZones()
    }

    function handleJoinZones() {
        var indicies = zoneList.getSelectedIndices();
        var zones = [];
        var coordinator = null;
        var model = null;
        if (indicies.length > 1) {
            for (var i = 0; i < indicies.length; i++) {
                model = AllZonesModel.get(indicies[i]);
                if (model.name === currentZone)
                    coordinator = model.payload
                else
                    zones.push(model.payload)
            }
            if (coordinator !== null) {
                var future = Sonos.tryJoinZones(zones, coordinator);
                future.finished.connect(mainView.actionFinished);
                return future.start();
            }
        }
        return false;
    }

    MultiSelectListView {
        id: zoneList
        anchors.fill: parent

        state: "selection"

        model: DelegateModel {
            id: visualModel
            model: AllZonesModel
            delegate: SelectMusicListItem {
                id: listItem
                listview: zoneList
                reorderable: false
                //selectable: true
                highlighted: (currentZone === model.name)

                color: "transparent"
                description: qsTr("Zone")

                onClicked: {
                    connectZone(model.name)
                }
                onAction3Pressed: {
                    if (zonePlayer.playbackState === "PLAYING") {
                        var future = zonePlayer.tryPause();
                        future.finished.connect(mainView.actionFinished);
                        future.start();
                    } else {
                        var future = zonePlayer.tryPlay();
                        future.finished.connect(mainView.actionFinished);
                        future.start();
                    }
                }
                action3Visible: false
                action3IconSource: ""
                onAction2Pressed: {
                    pageStack.push("qrc:/silica/pages/Group.qml", {"zoneId": model.id})
                }
                action2Visible: model.isGroup
                action2IconSource: model.isGroup ? "qrc:/images/edit-cut.svg" : ""
                onActionPressed: {
                    var future = Sonos.tryUnjoinZone(model.payload);
                    future.finished.connect(mainView.actionFinished);
                    future.start();
                }
                actionVisible: model.isGroup
                actionIconSource: model.isGroup ? "image://theme/icon-m-clear" : ""

                menu: ContextMenu {
                    hasContent: model.isGroup
                    MenuItem {
                        text: qsTr("Group")
                        enabled: model.isGroup
                        onClicked: {
                            pageStack.push("qrc:/silica/pages/Group.qml", {"zoneId": model.id})
                        }
                    }
                }

                contentHeight: units.gu(12)
                coverSize: units.gu(5)
                noCover: "qrc:/images/no_cover.png"

                property ZonePlayer zonePlayer: null
                property int pid: 0

                property string playbackState: ""
                property string currentMetaTitle: ""
                property string currentDuration: ""

                function handleZPSourceChanged() {
                    if (zonePlayer.currentProtocol === 1) {
                        imageSources = [{art: "qrc:/images/linein.png"}];
                    } else if (zonePlayer.currentProtocol === 5) {
                        imageSources = [{art: "qrc:/images/tv.png"}];
                    } else {
                        imageSources = makeCoverSource(zonePlayer.currentMetaArt, zonePlayer.currentMetaArtist, zonePlayer.currentMetaAlbum);
                    }
                    currentMetaTitle = zonePlayer.currentMetaTitle;
                    if (zonePlayer.currentTrackDuration > 0)
                        currentDuration = mainView.durationToString(1000 * zonePlayer.currentTrackDuration);
                    else
                        currentDuration = "";
                }

                function handleZPPlaybackStateChanged() {
                    if (zonePlayer.currentMetaSource === "") {
                        action3Visible = false;
                        action3IconSource = "";
                    } else {
                        action3Visible = true;
                        action3IconSource = (zonePlayer.playbackState === "PLAYING" ? "image://theme/icon-m-pause" : "image://theme/icon-m-play");
                    }
                    // translate the state
                    switch (zonePlayer.playbackState)
                    {
                    case "STOPPED": playbackState = qsTr("Stopped"); break;
                    case "PLAYING": playbackState = qsTr("Playing"); break;
                    case "PAUSED_PLAYBACK": playbackState = qsTr("Paused playback"); break;
                    case "TRANSITIONING": playbackState = qsTr("Transitioning"); break;
                    default: playbackState = zonePlayer.playbackState.replace('_', ' ');
                    }
                }

                Component.onCompleted: {
                    zonePlayer = AllZonesModel.holdPlayer(model.index);
                    if (zonePlayer) {
                        pid = zonePlayer.pid;
                        handleZPSourceChanged();
                        handleZPPlaybackStateChanged();
                    }
                }

                Component.onDestruction: {
                    if (zonePlayer)
                        AllZonesModel.releasePlayer(zonePlayer);
                }

                Connections {
                    target: AllZonesModel
                    onZpSourceChanged: { if (pid === listItem.pid) handleZPSourceChanged(); }
                    onZpPlaybackStateChanged: { if (pid === listItem.pid) handleZPPlaybackStateChanged(); }
                }

                column: Column {
                    spacing: units.gu(1)

                    Label {
                        id: zoneName
                        color: styleMusic.view.primaryColor
                        font.pixelSize: units.fx("large")
                        text: model.isGroup ? model.shortName : model.name
                    }

                    Label {
                        id: fullName
                        color: styleMusic.view.secondaryColor
                        font.pixelSize: units.fx("small")
                        text: model.isGroup ? model.name : playbackState
                        visible: text !== ""
                    }

                    Row {
                        spacing: units.gu(0.5)
                        width: parent.width

                        Label {
                            id: sourceTitle
                            color: styleMusic.view.primaryColor
                            font.pixelSize: units.fx("small")
                            text: currentMetaTitle
                            visible: currentMetaTitle !== ""
                        }

                        Label {
                            id: separatorDuration
                            color: styleMusic.view.primaryColor
                            font.pixelSize: units.fx("small")
                            text: " | "
                            visible: currentDuration !== ""
                        }

                        Label {
                            id: sourceDuration
                            color: styleMusic.view.primaryColor
                            font.pixelSize: units.fx("small")
                            text: currentDuration
                            visible: currentDuration !== ""
                        }
                    }
                }
            }
        }

        function selectCurrentZone() {
            var tmp = [];
            for (var i = 0; i < AllZonesModel.count; i++) {
                if (AllZonesModel.get(i).name === currentZone) {
                    tmp.push(i);
                    zoneList.selectedIndices = tmp;
                    break;
                }
            }
            synchronizeChecked();
        }

        // the current zone must be checked as it will be the controller of a new group
        // on initialization
        Component.onCompleted: {
            selectCurrentZone()
        }
        // on resetting model
        Connections {
            target: AllZonesModel
            onCountChanged: {
                zoneList.selectCurrentZone()
            }
        }
        // on change of the current zone
        Connections {
            target: mainView
            onZoneChanged: {
                zoneList.selectCurrentZone()
            }
        }
        // on deselecting
        onDeselected: {
            if (AllZonesModel.get(index).name === currentZone) {
                selectedIndices.push(index); // re-push the index
                synchronizeChecked();
            }
        }
    }
}
