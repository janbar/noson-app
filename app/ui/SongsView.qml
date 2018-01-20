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
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
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
    pageFlickable: songList

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

    width: mainPageStack.width

    property bool isFavorite: false

    state: songList.state === "multiselectable" ? "selection" : (isPlaylist ? "playlist" : "album")
    states: [
        AlbumSongsHeadState {
            thisPage: songStackPage
            thisHeader {
                extension: DefaultSections { }
            }
        },
        PlaylistHeadState {
            thisPage: songStackPage
            thisHeader {
                extension: DefaultSections { }
            }
        },
        MultiSelectHeadState {
            containerItem: songStackPage.containerItem
            listview: songList
            removable: isPlaylist
            thisPage: songStackPage
            thisHeader {
                extension: DefaultSections { }
            }

            onRemoved: {
                delayRemoveSelectedFromPlaylist.selectedIndices = selectedIndices
                delayRemoveSelectedFromPlaylist.start()
            }
        }
    ]

    Timer {
        id: delayRemoveSelectedFromPlaylist
        interval: 100
        property var selectedIndices: []
        onTriggered: {
            songList.focusIndex = selectedIndices[selectedIndices.length-1];
            if (removeTracksFromPlaylist(containerItem.id, selectedIndices, songsModel.containerUpdateID())) {
                songsModel.asyncLoad();
            }
        }
    }

    TracksModel {
        id: songsModel
        onDataUpdated: songsModel.asyncLoad()
        onLoaded: songsModel.resetModel()
        Component.onCompleted: {
            songsModel.init(Sonos, songSearch, false)
            songsModel.asyncLoad()
        }
    }

    function restoreFocusIndex() {
        if (songsModel.count <= songList.focusIndex) {
            songsModel.asyncLoadMore() // load more !!!
        } else {
            songList.positionViewAtIndex(songList.focusIndex, ListView.Center);
            songList.focusIndex = -1
        }
    }

    Connections {
        target: songsModel
        onDataUpdated: songsModel.asyncLoad()
        onLoaded: {
            songsModel.resetModel()
            if (succeeded) {
                if (songList.focusIndex > 0) {
                    // restore index position in view
                    restoreFocusIndex()
                }
            }
        }
        onLoadedMore: {
            if (succeeded) {
                songsModel.appendModel();
                if (songList.focusIndex > 0) {
                    // restore index position in view
                    restoreFocusIndex()
                }
            } else if (songList.focusIndex > 0) {
                songList.positionViewAtEnd();
                songList.focusIndex = -1;
            }
        }

    }

    Repeater {
        id: songModelRepeater
        model: songsModel

        delegate: Item {
            property string art: model.art
            property string artist: model.author
            property string album: model.album
        }
        property bool hasCover: covers.length ? true : false

        onItemAdded: {
            if (!hasCover && item.art !== "") {
                songStackPage.covers = [{art: item.art}]
                hasCover = true
            }
        }
    }

    BlurredBackground {
            id: blurredBackground
            height: parent.height
    }

    MultiSelectListView {
        id: songList
        anchors {
            fill: parent
        }
        width: parent.width
        header: MusicHeader {
            id: blurredHeader
            rightColumn: Column {
                spacing: units.gu(2)
                ShuffleButton {
                    model: songList.model
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
            coverFlow: 4
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
                    text: line2
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
                    text: line1
                    visible: line1 !== ""
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
                          ? (year !== "" ? year + " | " : "") + i18n.tr("%1 song", "%1 songs", songsModel.totalCount).arg(songsModel.totalCount)
                          : i18n.tr("%1 song", "%1 songs", songsModel.totalCount).arg(songsModel.totalCount)
                    wrapMode: Text.NoWrap
                }
            }

            onFirstSourceChanged: {
                blurredBackground.art = firstSource
            }
        }

        model: songsModel

        property int focusIndex: 0

        delegate: MusicListItem {
            id: track
            color: "transparent"
            noCover: Qt.resolvedUrl("../graphics/no_cover.png")
            imageSource: !songStackPage.isAlbum ? makeCoverSource(model.art, model.author, model.album) : noCover
            column: Column {
                Label {
                    id: trackTitle
                    color: styleMusic.common.music
                    fontSize: !songStackPage.isAlbum ? "small" : "medium"
                    objectName: "songspage-tracktitle"
                    text: model.title
                }

                Label {
                    id: trackArtist
                    color: styleMusic.common.subtitle
                    fontSize: !songStackPage.isAlbum ? "x-small" : "small"
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
                    },
                    AddToFavorites {
                        description: i18n.tr("Song")
                        art: model.art
                    }
                ]
                delegate: ActionDelegate {
                }
            }

            onImageError: model.art = "" // reset invalid url from model
            onItemClicked: currentDialog = dialogSongInfo.open(model, true) // show actions play...

            Loader {
                id: playlistRemoveAction
                sourceComponent: ListItemActions {
                    actions: [
                        Remove {
                            onTriggered: {
                                delayRemoveTrackFromPlaylist.start()
                            }
                        }
                    ]
                }
            }

            Timer {
                id: delayRemoveTrackFromPlaylist
                interval: 100
                onTriggered: {
                    songList.focusIndex = index > 0 ? index - 1 : 0;
                    if (removeTracksFromPlaylist(containerItem.id, [index], songsModel.containerUpdateID())) {
                        songsModel.asyncLoad();
                    }
                }
            }

            Component.onCompleted: {
                if (model.date !== undefined) {
                    songStackPage.year = new Date(model.date).toLocaleString(Qt.locale(),'yyyy');
                }
            }
        }

        onReorder: {
            customdebug("Reorder queue item " + from + " to " + to);
            songList.focusIndex = to
            mainView.jobRunning = true
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
                    songsModel.asyncLoad();
                }
            }
        }

        onAtYEndChanged: {
            if (songList.atYEnd && songsModel.totalCount > songsModel.count) {
                songsModel.asyncLoadMore()
            }
        }

    }

    Scrollbar {
        flickableItem: songList
        align: Qt.AlignTrailing
    }

    // check favorite on data loaded
    Connections {
        target: AllFavoritesModel
        onCountChanged: {
            isFavorite = (AllFavoritesModel.findFavorite(containerItem.payload).length > 0)
        }
    }

    Component.onCompleted: {
        isFavorite = (AllFavoritesModel.findFavorite(containerItem.payload).length > 0)
    }
}
