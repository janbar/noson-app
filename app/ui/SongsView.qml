/*
 * Copyright (C) 2013, 2014, 2015, 2016
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Andrew Hayzen <ahayzen@gmail.com>
 *      Daniel Holm <d.holmen@gmail.com>
 *      Victor Thompson <victor.thompson@gmail.com>
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
import Ubuntu.Thumbnailer 0.1
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"
import "../components/ListItemActions"
import "../components/ViewButton"
import "../components/Dialog"

MusicPage {
    id: songStackPage
    objectName: "songsPage"
    visible: false

    property string line1: ""
    property string line2: ""
    property var covers: []
    property bool isAlbum: false
    property bool isPlaylist: false
    property string year: ""

    property var containerItem: null

    property string songSearch: ""
    property string album: ""
    property string artist: ""
    property string genre: ""

    property bool loaded: false  // used to detect difference between first and further loads

    state: albumtrackslist.state === "multiselectable" ? "selection" : (isPlaylist ? "playlist" : "album")
    states: [
        PageHeadState {
            id: albumState
            name: "album"
            PropertyChanges {
                target: songStackPage.head
                backAction: albumState.backAction
                actions: albumState.actions
            }
        },
        PageHeadState {
            id: playlistState
            name: "playlist"
            actions: [
                /*Action {
                    objectName: "editPlaylist"
                    iconName: "edit"
                    onTriggered: {
                        currentDialog = PopupUtils.open(Qt.resolvedUrl("../components/Dialog/DialogEditPlaylist.qml"), mainView)
                        currentDialog.oldPlaylistName = line2
                    }
                    enabled: false
                },*/
                Action {
                    objectName: "deletePlaylist"
                    iconName: "delete"
                    onTriggered: {
                        currentDialog = PopupUtils.open(Qt.resolvedUrl("../components/Dialog/DialogRemovePlaylist.qml"), mainView)
                        currentDialog.oldPlaylistId = songStackPage.containerItem.id
                    }
                }
            ]
            PropertyChanges {
                target: songStackPage.head
                backAction: playlistState.backAction
                actions: playlistState.actions
            }
        },
        MultiSelectHeadState {
            id: selection
            containerItem: songStackPage.containerItem
            listview: albumtrackslist
            removable: isPlaylist
            thisPage: songStackPage

            onRemoved: {
                if (removeTracksFromPlaylist(containerItem.id, selectedIndices, songsModel.containerUpdateID())) {
                    songsModel.load(); // FIXME: no event
                }
            }
        }
    ]

    TracksModel {
        id: songsModel
    }

    Timer {
        id: delayInitTrackModel
        interval: 100
        onTriggered: {
            songsModel.init(Sonos, songSearch, true)
            mainView.currentlyWorking = false
            songStackPage.loaded = true;
        }
    }

    Connections {
        target: songsModel
        onDataUpdated: {
            mainView.currentlyWorking = true
            delayLoadTrackModel.start()
        }
    }

    Timer {
        id: delayLoadTrackModel
        interval: 100
        onTriggered: {
            songsModel.load()
            mainView.currentlyWorking = false
        }
    }

    Repeater {
        id: songModelRepeater
        model: songsModel

        delegate: Item {
            property string art: model.art
        }
        property var arts: []
        property bool hasCover: covers.length ? true : false

        onItemAdded: {
            if (!hasCover && item.art !== "") {
                covers = [{art: item.art}]
                hasCover = true
            }
        }
    }

    MultiSelectListView {
        id: albumtrackslist
        anchors {
            fill: parent
        }
        objectName: "songspage-listview"
        width: parent.width

        header: BlurredHeader {
            id: blurredHeader
            rightColumn: Column {
                spacing: units.gu(2)
                ShuffleButton {
                    model: albumtrackslist.model
                    width: blurredHeader.width > units.gu(60) ? units.gu(23.5) : (blurredHeader.width - units.gu(13)) / 2
                }
                QueueAllButton {
                    containerItem: songStackPage.containerItem
                    width: blurredHeader.width > units.gu(60) ? units.gu(23.5) : (blurredHeader.width - units.gu(13)) / 2
                    visible: containerItem ? true : false
                }
                PlayAllButton {
                    containerItem: songStackPage.containerItem
                    width: blurredHeader.width > units.gu(60) ? units.gu(23.5) : (blurredHeader.width - units.gu(13)) / 2
                    visible: containerItem ? true : false

                }
            }
            property int baseHeight: songStackPage.width > units.gu(60) ? units.gu(33.5) : ((songStackPage.width - units.gu(5)) / 2) + units.gu(12)
            coverSources: songStackPage.covers
            height: isAlbum && songStackPage.width <= units.gu(60) ?
                        baseHeight + units.gu(3) : baseHeight
            bottomColumn: Column {
                Label {
                    id: albumLabel
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    color: styleMusic.common.music
                    elide: Text.ElideRight
                    fontSize: "x-large"
                    maximumLineCount: 1
                    text: line2 != "" ? line2 : i18n.tr("Unknown Album")
                    wrapMode: Text.NoWrap
                }

                Item {
                    height: units.gu(0.75)
                    width: parent.width
                    visible: albumArtist.visible
                }

                Label {
                    id: albumArtist
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    color: styleMusic.common.subtitle
                    elide: Text.ElideRight
                    fontSize: "small"
                    maximumLineCount: 1
                    objectName: "songsPageHeaderAlbumArtist"
                    text: line1 != "" ? line1 : i18n.tr("Unknown Artist")
                    visible: isAlbum
                    wrapMode: Text.NoWrap
                }

                Item {
                    height: units.gu(1)
                    width: parent.width
                }

                Label {
                    id: albumYear
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    color: styleMusic.common.subtitle
                    elide: Text.ElideRight
                    fontSize: "small"
                    maximumLineCount: 1
                    text: isAlbum
                          ? (year !== "" ? year + " | " : "") + i18n.tr("%1 song", "%1 songs", albumtrackslist.count).arg(albumtrackslist.count)
                          : i18n.tr("%1 song", "%1 songs", albumtrackslist.count).arg(albumtrackslist.count)
                    wrapMode: Text.NoWrap
                }
            }
        }

        model: songsModel

        delegate: MusicListItem {
            id: track
            objectName: "songsPageListItem" + index
            imageSource: songStackPage.isPlaylist ? model : undefined
            column: Column {
                Label {
                    id: trackTitle
                    color: styleMusic.common.music
                    fontSize: "small"
                    objectName: "songspage-tracktitle"
                    text: model.title
                }

                Label {
                    id: trackArtist
                    color: styleMusic.common.subtitle
                    fontSize: "x-small"
                    text: model.author
                }
            }
            leadingActions: isPlaylist ? playlistRemoveAction.item : null
            multiselectable: true
            reorderable: isPlaylist
            trailingActions: ListItemActions {
                actions: [
                    PlaySong {
                    },
                    AddToQueue {
                    },
                    AddToPlaylist {
                    }
                ]
                delegate: ActionDelegate {
                }
            }

            onItemClicked: currentDialog = dialogSongInfo.open(model, true) // show actions play...

            Loader {
                id: playlistRemoveAction
                sourceComponent: ListItemActions {
                    actions: [
                        Remove {
                            onTriggered: {
                                mainView.currentlyWorking = true
                                delayRemoveTracksFromPlaylist.start()
                            }
                        }
                    ]
                }
            }

            Timer {
                id: delayRemoveTracksFromPlaylist
                interval: 100
                onTriggered: {
                    if (removeTracksFromPlaylist(containerItem.id, [index], songsModel.containerUpdateID())) {
                        songsModel.load();
                    }
                    mainView.currentlyWorking = false
                }
            }

            Component.onCompleted: {
                if (model.date !== undefined) {
                    songStackPage.year = new Date(model.date).toLocaleString(Qt.locale(),'yyyy');
                }
            }
        }

        onReorder: {
            mainView.currentlyWorking = true
            delayReorderTrackInPlaylist.argFrom = from
            delayReorderTrackInPlaylist.argTo = to
            delayReorderTrackInPlaylist.start()
        }

        Timer {
            id: delayReorderTrackInPlaylist
            interval: 100
            property int argFrom: 0
            property int argTo: 0
            onTriggered: {
                if (reorderTrackInPlaylist(containerItem.id, argFrom, argTo, songsModel.containerUpdateID())) {
                    songsModel.load();
                }
                mainView.currentlyWorking = false
            }
        }
    }

    Component.onCompleted: {
        mainView.currentlyWorking = true
        delayInitTrackModel.start()
    }
}
