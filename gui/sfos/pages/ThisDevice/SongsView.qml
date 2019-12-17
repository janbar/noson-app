/*
 * Copyright (C) 2019
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

import QtQuick 2.2
import Sailfish.Silica 1.0

import QtQml.Models 2.3
import NosonApp 1.0
import NosonMediaScanner 1.0
import "../../components"
import "../../components/Delegates"
import "../../components/Flickables"
import "../../components/Dialog"

MusicPage {
    id: songStackPage
    objectName: "songsPage"
    visible: false
    pageFlickable: songList
    isListView: true
    listview: songList

    property string line1: ""
    property string line2: ""
    property var covers: []
    property bool isAlbum: false
    property string year: ""
    property string album: ""
    property string artist: ""
    property string genre: ""

    // enable selection toolbar
    selectable: true

    onStateChanged: {
        if (state === "selection")
            songList.state = "selection"
        else
            songList.state = "default"
    }

    TrackList {
        id: tracks
        album: songStackPage.album
        artist: songStackPage.artist
        genre: songStackPage.genre
        Component.onCompleted: init()
    }

    SortFilterModel {
        id: songsModel
        model: tracks
        sort.property: isAlbum ? "albumTrackNo" : "normalized"
        sort.order: Qt.AscendingOrder
        sortCaseSensitivity: Qt.CaseInsensitive
    }

    function makeFileCoverSource(modelItem) {
        var art = "";
        if (modelItem.hasArt)
            art = player.makeFilePictureLocalURL(modelItem.filePath);
        return makeCoverSource(art, modelItem.author, modelItem.album);
    }

    function makeItemPayload(modelItem) {
        return player.makeFileStreamItem(modelItem.filePath, modelItem.codec, modelItem.title, modelItem.album,
                                         modelItem.author, modelItem.duration.toString(), modelItem.hasArt);
    }

    function makeContainerPayloads(model) {
        var items = [];
        for (var i = 0; i < model.count; i++) {
            var item = model.get(i);
            items.push({id: item.id, payload: makeItemPayload(item)});
        }
        return items;
    }

    Repeater {
        id: songModelRepeater
        model: songsModel

        delegate: Item {
            property string art: model.art
            property string artist: model.author
            property string album: model.album
            property string filePath: model.filePath
            property bool hasArt: model.hasArt
        }
        property bool hasCover: covers.length ? true : false

        onItemAdded: {
            if (!hasCover) {
                if (item.art !== "") {
                    songStackPage.covers = [{art: item.art}];
                    hasCover = true;
                } else if (item.hasArt) {
                    item.art = player.makeFilePictureLocalURL(item.filePath);
                    songStackPage.covers = [{art: item.art}];
                    hasCover = true;
                }
            }
        }
    }

    Component {
        id: dragDelegate

        SelectMusicListItem {
            id: listItem
            listview: songList
            reorderable: false
            selectable: true

            onClick: {
                var item = {
                    id: model.id, title: model.title, album: model.album, author: model.author, albumTrackNo: model.albumTrackNo,
                    payload: makeItemPayload(model)
                };
                if (isAlbum)
                    // header covers
                    dialogSongInfo.open(item, covers, true, true); // show actions
                else
                    // item cover
                    dialogSongInfo.open(item, [{art: imageSource}], true, true); // show actions
            }

            color: "transparent"

            noCover: !songStackPage.isAlbum ? "qrc:/images/no_cover.png" : "qrc:/images/no_cover.png"
            imageSources: !songStackPage.isAlbum ? makeFileCoverSource(model) : [{art: "qrc:/images/no_cover.png"}]
            description: qsTr("Song")

            onImageError: model.art = "" // reset invalid url from model
            onActionPressed: {
                var payload = makeItemPayload(model);
                trackClicked({id: model.id, payload: payload}, true);
            }
            actionVisible: true
            actionIconSource: "qrc:/images/media-preview-start.svg"
            menuVisible: true

            menuItems: [
                AddToQueue {
                    modelItem: model
                }
            ]

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
                    visible: !isAlbum
                }

                Label {
                    id: trackInfo
                    color: styleMusic.view.secondaryColor
                    font.pixelSize: units.fx(isAlbum ? "small" : "x-small")
                    text: model.codec + " " + (model.bitRate > 999999 ? model.sampleRate : model.bitRate > 0 ? Math.round(model.bitRate/1000) + "k" : "")
                }
            }

            Component.onCompleted: {
                if (model.year > 0) {
                    songStackPage.year = model.year
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
            rightColumn: Column {
                spacing: units.gu(1)
                ShuffleButton {
                    model: songsModel
                    width: units.gu(24)
                }
                QueueAllButton {
                    model: songsModel
                    width: units.gu(24)
                }
                PlayAllButton {
                    model: songsModel
                    width: units.gu(24)
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
                          ? (year !== "" ? year + " | " : "") + qsTr("%n song(s)", "", songsModel.count)
                          : qsTr("%n song(s)", "", songsModel.count)
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

        Connections {
            target: songStackPage
            onSelectAllClicked: songList.selectAll()
            onSelectNoneClicked: songList.selectNone()
            onAddToQueueClicked: {
                var indicies = songList.getSelectedIndices();
                var items = [];
                for (var i = 0; i < indicies.length; i++) {
                    var itemModel = songsModel.get(indicies[i]);
                    var payload = makeItemPayload(itemModel);
                    items.push({id: itemModel.id, payload: payload});
                }
                if (addMultipleItemsToQueue(items)) {
                    songList.selectNone()
                    songStackPage.state = "default"
                }
            }
        }

        onHasSelectionChanged: {
            songStackPage.selectAllVisible = !hasSelection
            songStackPage.selectNoneVisible = hasSelection
            songStackPage.addToQueueVisible = hasSelection
            songStackPage.addToPlaylistVisible = false
            songStackPage.removeSelectedVisible = false

        }

        Component.onCompleted: {
            songStackPage.selectAllVisible = !hasSelection
            songStackPage.selectNoneVisible = hasSelection
            songStackPage.addToQueueVisible = hasSelection
            songStackPage.addToPlaylistVisible = false
            songStackPage.removeSelectedVisible = false
        }
    }
}
