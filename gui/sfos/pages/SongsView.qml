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
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/ListItemActions"
import "../components/ViewButton"
import "../components/Dialog"

MusicPage {
    id: songStackPage
    objectName: "songsPage"
    visible: false
    pageFlickable: songList
    pageMenuEnabled: true
    isListView: true
    listview: songList

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

    property bool isFavorite: false

    // enable selection toolbar
    selectable: true

    onStateChanged: {
        if (state === "selection")
            songList.state = "selection"
        else
            songList.state = "default"
    }

    Timer {
        id: delayRemoveSelectedFromPlaylist
        interval: 100
        property var selectedIndices: []
        onTriggered: {
            songList.focusIndex = selectedIndices[selectedIndices.length-1];
            if (!removeTracksFromPlaylist(containerItem.id, selectedIndices, songsModel.containerUpdateID(), function(result) {
                if (result) {
                    popInfo.open(qsTr("%n song(s) removed", "", selectedIndices.length));
                } else {
                    mainView.actionFailed();
                }
            })) {
                mainView.actionFailed();
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

    Component {
        id: dragDelegate

        SelectMusicListItem {
            id: listItem
            listview: songList
            //reorderable: isPlaylist

            onClicked: {
                var arts = [];
                if (isAlbum)
                    arts = covers; // header covers
                else
                    arts = [{art: imageSource}]; // item cover

                dialogSongInfo.open(model, arts,
                                    "qrc:/sfos/pages/ArtistView.qml",
                                    {
                                        "artistSearch": "A:ARTIST/" + model.author,
                                        "artist": model.author,
                                        "covers": makeCoverSource(undefined, model.author, undefined),
                                        "pageTitle": qsTr("Artist")
                                    },
                                    true,   // force show more
                                    true,   // can play
                                    true    // can queue
                                    );
            }

            color: "transparent"

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onCountChanged: {
                    listItem.isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            noCover: "qrc:/images/no_cover.png"
            rowNumber: songStackPage.isAlbum ? model.albumTrackNo !== "" ? model.albumTrackNo : "#" : ""
            imageSources: !songStackPage.isAlbum ? makeCoverSource(model.art, model.author, model.album) : []
            description: qsTr("Song")

            onImageError: model.art = "" // reset invalid url from model
            onActionPressed: trackClicked(model) // play track
            actionVisible: true
            actionIconSource: "image://theme/icon-m-play"

            menu: ContextMenu {
                AddToFavorites {
                    isFavorite: listItem.isFavorite
                    description: listItem.description
                    art: model.art
                }
                AddToPlaylist {
                }
                AddToQueue {
                }
                Remove {
                    enabled: isPlaylist
                    visible: enabled
                    onClicked: {
                        listview.focusIndex = index > 0 ? index - 1 : 0;
                        delayRemoveSelectedFromPlaylist.selectedIndices = [index]
                        delayRemoveSelectedFromPlaylist.start()
                        listItem.color = "red";
                    }
                }
            }

            coverSize: units.gu(5)

            column: Column {
                Label {
                    id: trackTitle
                    color: styleMusic.view.primaryColor
                    font.pixelSize: units.fx("medium")
                    text: model.title
                }

                Label {
                    id: trackArtist
                    color: styleMusic.view.secondaryColor
                    font.pixelSize: units.fx("x-small")
                    text: model.author
                }
            }

            Component.onCompleted: {
                isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                if (model.date !== undefined) {
                    songStackPage.year = new Date(model.date).toLocaleString(Qt.locale(),'yyyy')
                }
            }
        }
    }

    MultiSelectListView {
        id: songList
        anchors.fill: parent
        clip: true

        header: MusicHeader {
            id: blurredHeader
            isFavorite: songStackPage.isFavorite
            rightColumn: Column {
                spacing: units.gu(1)
                ShuffleButton {
                    model: songsModel
                    width: units.gu(24)
                }
                QueueAllButton {
                    containerItem: songStackPage.containerItem
                    width: units.gu(24)
                    visible: containerItem ? true : false
                }
                PlayAllButton {
                    containerItem: songStackPage.containerItem
                    width: units.gu(24)
                    visible: containerItem ? true : false

                }
            }
            height: contentHeight
            coverSources: songStackPage.covers
            coverFlow: 4
            titleColumn: Column {
                spacing: units.gu(1)

                Label {
                    id: albumLabel
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    color: styleMusic.view.foregroundColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("x-large")
                    maximumLineCount: 1
                    text: line2
                    wrapMode: Text.NoWrap
                }

                Label {
                    id: albumArtist
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    color: styleMusic.view.secondaryColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("small")
                    maximumLineCount: 1
                    text: line1
                    visible: line1 !== ""
                    wrapMode: Text.NoWrap
                }

                Label {
                    id: albumYear
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    color: styleMusic.view.secondaryColor
                    elide: Text.ElideRight
                    font.pixelSize: units.fx("small")
                    maximumLineCount: 1
                    text: isAlbum
                          ? (year !== "" ? year + " | " : "") + qsTr("%n song(s)", "", songsModel.totalCount)
                          : qsTr("%n song(s)", "", songsModel.totalCount)
                    wrapMode: Text.NoWrap
                }

                Item {
                    id: spacer
                    width: parent.width
                    height: units.gu(1)
                }
            }
        }

        model: DelegateModel {
            id: visualModel
            model: songsModel
            delegate: dragDelegate
        }

        property int focusIndex: 0

        signal reorder(int from, int to)

        onReorder: {
            customdebug("Reorder item " + from + " to " + to);
            songList.focusIndex = to;
            if (!reorderTrackInPlaylist(containerItem.id, from, to, songsModel.containerUpdateID(), function(result) {
                if (!result) {
                    songsModel.asyncLoad();
                    mainView.actionFailed();
                }
            })) {
                songsModel.asyncLoad();
                mainView.actionFailed();
            }
        }

        onAtYEndChanged: {
            if (songList.atYEnd && songsModel.totalCount > songsModel.count) {
                songsModel.asyncLoadMore()
            }
        }

        Connections {
            target: songStackPage
            onSelectAllClicked: songList.selectAll()
            onSelectNoneClicked: songList.selectNone()
            onAddToQueueClicked: {
                var indicies = songList.getSelectedIndices();
                // when select all then add the container if exists
                if (containerItem && indicies.length === songsModel.count) {
                    if (addQueue(containerItem)) {
                        songList.selectNone()
                        songStackPage.state = "default"
                    }
                }
                else {
                    var items = [];
                    for (var i = 0; i < indicies.length; i++) {
                        items.push(songsModel.get(indicies[i]));
                    }
                    if (addMultipleItemsToQueue(items)) {
                        songList.selectNone()
                        songStackPage.state = "default"
                    }
                }
            }
            onAddToPlaylistClicked: {
                var items = []
                var indicies = songList.getSelectedIndices();
                // when select all then add the container if exists
                if (containerItem && indicies.length === songsModel.count)
                    items.push(containerItem);
                else {
                    for (var i = 0; i < indicies.length; i++) {
                        items.push({"payload": songsModel.get(indicies[i]).payload});
                    }
                }
                pageStack.push("qrc:/sfos/pages/AddToPlaylist.qml", {"chosenElements": items})
                songList.selectNone()
                songStackPage.state = "default"
            }

            onRemoveSelectedClicked: {
                delayRemoveSelectedFromPlaylist.selectedIndices = songList.getSelectedIndices()
                if (delayRemoveSelectedFromPlaylist.selectedIndices.length > 0)
                    songList.focusIndex = delayRemoveSelectedFromPlaylist.selectedIndices[delayRemoveSelectedFromPlaylist.selectedIndices.length - 1]
                delayRemoveSelectedFromPlaylist.start()
                songList.selectNone()
                songStackPage.state = "default"
            }
        }

        onHasSelectionChanged: {
            songStackPage.selectAllVisible = !hasSelection
            songStackPage.selectNoneVisible = hasSelection
            songStackPage.addToQueueVisible = hasSelection
            songStackPage.addToPlaylistVisible = hasSelection
            songStackPage.removeSelectedVisible = isPlaylist && hasSelection
        }

        Component.onCompleted: {
            songStackPage.selectAllVisible = !hasSelection
            songStackPage.selectNoneVisible = hasSelection
            songStackPage.addToQueueVisible = hasSelection
            songStackPage.addToPlaylistVisible = hasSelection
            songStackPage.removeSelectedVisible = isPlaylist && hasSelection
        }
    }

    Component {
        id: menuItemComp
        MenuItem {
        }
    }

    property MenuItem menuRemovePlaylist: null
    function onMenuRemovePlaylist() {
        dialogRemovePlaylist.open(songStackPage.containerItem)
        dialogRemovePlaylist.onDone.connect(afterRemovedPlaylist.start)
    }
    Timer {
        id: afterRemovedPlaylist
        interval: 50
        onTriggered: {
            if (dialogRemovePlaylist.result === DialogResult.Accepted) {
                if (dialogRemovePlaylist.status !== DialogStatus.Closed)
                    restart();
                else
                    pageStack.pop();
            }
        }
    }

    property MenuItem menuAddItemToFavorites: null
    property MenuItem menuRemoveFromFavorites: null
    onIsFavoriteChanged: {
        if (containerItem) {
            if (menuAddItemToFavorites)
                menuAddItemToFavorites.visible = !songStackPage.isFavorite;
            if (menuRemoveFromFavorites)
                menuRemoveFromFavorites.visible = songStackPage.isFavorite;
        }
    }

    function onMenuAddItemToFavorites() {
        if (addItemToFavorites(containerItem, pageTitle, songList.headerItem.firstSource))
            songStackPage.isFavorite = true
    }
    function onMenuRemoveFromFavorites() {
        if (removeFromFavorites(containerItem.payload))
            songStackPage.isFavorite = false
    }

    // check favorite on data loaded
    Connections {
        target: AllFavoritesModel
        onCountChanged: {
            if (containerItem)
                isFavorite = (AllFavoritesModel.findFavorite(containerItem.payload).length > 0)
        }
    }

    Component.onCompleted: {
        if (containerItem) {
            isFavorite = (AllFavoritesModel.findFavorite(containerItem.payload).length > 0)

            console.debug("Populating menu")
            menuAddItemToFavorites = menuItemComp.createObject(pageMenuContent, {"text" : qsTr("Add to favorites"), "visible" : !songStackPage.isFavorite})
            menuAddItemToFavorites.onClicked.connect(onMenuAddItemToFavorites)
            menuRemoveFromFavorites = menuItemComp.createObject(pageMenuContent, {"text" : qsTr("Remove from favorites"), "visible" : songStackPage.isFavorite})
            menuRemoveFromFavorites.onClicked.connect(onMenuRemoveFromFavorites)
            menuRemovePlaylist = menuItemComp.createObject(pageMenuContent, {"text" : qsTr("Delete"), "visible" : songStackPage.isPlaylist})
            menuRemovePlaylist.onClicked.connect(onMenuRemovePlaylist)
        }
    }
}
