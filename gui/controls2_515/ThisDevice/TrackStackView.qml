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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQml.Models 2.3
import NosonApp 1.0
import NosonMediaScanner 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/Dialog"

MusicPage {
    id: trackStackPage
    objectName: "trackStackPage"
    visible: false
    pageFlickable: songList
    isListView: true
    listview: songList

    property string line1: ""
    property string line2: ""
    property var covers: []
    property int coverFlow: 1
    property string noCover: ""
    property bool isAlbum: false
    property string year: ""
    property string album: ""
    property string albumArtist: ""
    property string artist: ""
    property string genre: ""
    property string composer: ""

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
        album: trackStackPage.album
        albumArtist: trackStackPage.albumArtist
        artist: trackStackPage.artist
        genre: trackStackPage.genre
        composer: trackStackPage.composer
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
        return makeCoverSource(art, modelItem.albumArtist, modelItem.album);
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

//    Repeater {
//        id: songModelRepeater
//        model: songsModel

//        delegate: Item {
//            property string art: model.art
//            property string artist: model.author
//            property string album: model.album
//            property string filePath: model.filePath
//            property bool hasArt: model.hasArt
//        }
//        property bool hasCover: covers.length ? true : false

//        onItemAdded: {
//            if (!hasCover) {
//                if (item.art !== "") {
//                    songStackPage.covers = [{art: item.art}];
//                    hasCover = true;
//                } else if (item.hasArt) {
//                    item.art = player.makeFilePictureLocalURL(item.filePath);
//                    songStackPage.covers = [{art: item.art}];
//                    hasCover = true;
//                }
//            }
//        }
//    }

    BlurredBackground {
        id: blurredBackground
        height: parent.height
    }

    header: Column {
        width: parent.width
        height: implicitHeight
        Row {
            id: listHeader
            width: parent.width
            height: units.gu(7)
            property real childSize: Math.min((width - units.gu(4)) / 3, units.gu(16))
            spacing: units.gu(1)
            leftPadding: units.gu(1)
            rightPadding: units.gu(1)
            ShuffleButton {
                height: units.gu(5)
                width: listHeader.childSize
                anchors.verticalCenter: parent.verticalCenter
                model: songsModel
            }
            PlayAllButton {
                height: units.gu(5)
                width: listHeader.childSize
                anchors.verticalCenter: parent.verticalCenter
                model: songsModel
            }
            QueueAllButton {
                height: units.gu(5)
                width: listHeader.childSize
                anchors.verticalCenter: parent.verticalCenter
                model: songsModel
            }
        }
        Row {
            width: parent.width
            height: coverGrid.visible ? coverGrid.height + units.gu(1) : headerInfo.height + units.gu(1)
            spacing: units.gu(1)
            leftPadding: spacing
            Rectangle {
                id: coverGrid
                anchors.verticalCenter: parent.verticalCenter
                height: units.gu(9)
                width: height
                border.color: "grey"
                color: "transparent"
                CoverGrid {
                    id: coversImage
                    anchors.verticalCenter: parent.verticalCenter
                    size: parent.height
                    covers: trackStackPage.covers
                    flowModel: trackStackPage.coverFlow
                    noCover: trackStackPage.noCover
                    onFirstSourceChanged: {
                        blurredBackground.art = firstSource
                    }
                }
                visible: covers.length > 0
            }
            Item {
                id: headerInfo
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - coverGrid.width - units.gu(3)
                height: headerInfoContent.height
                Column {
                    id: headerInfoContent
                    width: parent.width
                    height: implicitHeight
                    spacing: units.gu(1)
                    Label {
                        id: albumLabel
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        color: styleMusic.view.foregroundColor
                        elide: Text.ElideRight
                        font.pointSize: units.fs("x-large")
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
                        font.pointSize: units.fs("small")
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
                        font.pointSize: units.fs("small")
                        maximumLineCount: 1
                        text: isAlbum
                              ? (year !== "" ? year + " | " : "") + qsTr("%n song(s)", "", songsModel.count)
                              : qsTr("%n song(s)", "", songsModel.count)
                        wrapMode: Text.NoWrap
                    }
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
                var arts = [];
                if (isAlbum)
                    arts = covers; // header covers
                else
                    arts = [{art: imageSource}]; // item cover

                dialogSongInfo.open(item, arts,
                                    "qrc:/controls2/ThisDevice/ArtistView.qml",
                                    {
                                        "artist": model.author,
                                        "covers": makeCoverSource(undefined, model.author, undefined),
                                        "pageTitle": pageTitle
                                    },
                                    !stackView.objectInStack("artistViewPage"), // force show more
                                    true,   // can play
                                    true    // can queue
                                    );
            }

            color: "transparent"

            noCover: "qrc:/images/no_cover.png"
            rowNumber: trackStackPage.isAlbum ? model.albumTrackNo > 0 ? model.albumTrackNo.toString() : "#" : ""
            imageSources: !trackStackPage.isAlbum ? makeFileCoverSource(model) : []
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
                    font.pointSize: units.fs("medium")
                    text: model.title
                }

                Label {
                    id: trackArtist
                    color: styleMusic.view.secondaryColor
                    font.pointSize: units.fs("x-small")
                    text: model.author
                    visible: !isAlbum
                }

                Label {
                    id: trackInfo
                    color: styleMusic.view.secondaryColor
                    font.pointSize: units.fs(isAlbum ? "small" : "x-small")
                    text: model.codec + " " + (model.bitRate > 999999 ? model.sampleRate : model.bitRate > 0 ? Math.round(model.bitRate/1000) + "k" : "")
                }
            }

            Component.onCompleted: {
                if (model.year > 0) {
                    trackStackPage.year = model.year
                }
            }
        }
    }

    MultiSelectListView {
        id: songList
        anchors.fill: parent
        clip: true

        model: DelegateModel {
            id: visualModel
            model: songsModel
            delegate: dragDelegate
        }

        Connections {
            target: trackStackPage
            function onSelectAllClicked() {
                songList.selectAll()
            }
            function onSelectNoneClicked() {
                songList.selectNone()
            }
            function onAddToQueueClicked() {
                var indicies = songList.getSelectedIndices();
                var items = [];
                for (var i = 0; i < indicies.length; i++) {
                    var itemModel = songsModel.get(indicies[i]);
                    var payload = makeItemPayload(itemModel);
                    items.push({id: itemModel.id, payload: payload});
                }
                if (addMultipleItemsToQueue(items)) {
                    songList.selectNone()
                    trackStackPage.state = "default"
                }
            }
        }

        onHasSelectionChanged: {
            trackStackPage.selectAllVisible = !hasSelection
            trackStackPage.selectNoneVisible = hasSelection
            trackStackPage.addToQueueVisible = hasSelection
            trackStackPage.addToPlaylistVisible = false
            trackStackPage.removeSelectedVisible = false

        }

        Component.onCompleted: {
            trackStackPage.selectAllVisible = !hasSelection
            trackStackPage.selectNoneVisible = hasSelection
            trackStackPage.addToQueueVisible = hasSelection
            trackStackPage.addToPlaylistVisible = false
            trackStackPage.removeSelectedVisible = false
        }
    }
}
