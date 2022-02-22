/*
 * Copyright (C) 2016, 2022
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
import "components/ListItemActions"
import "components/Dialog"

MusicPage {
    id: tracksPage
    objectName: "tracksPage"
    visible: false
    pageFlickable: trackList
    isListView: true
    listview: trackList

    property var covers: []
    property var containerItem: null
    property string containerTitle: ""
    property string containerId: ""

    property bool isFavorite: false

    // enable selection toolbar
    selectable: true

    // focus index
    property int focusIndex: -1

    onStateChanged: {
        if (state === "selection")
            trackList.state = "selection"
        else
            trackList.state = "default"
    }

    Timer {
        id: delayRemoveSelectedFromPlaylist
        interval: 100
        property var selectedIndices: []
        onTriggered: {
            focusIndex = selectedIndices[selectedIndices.length-1];
            if (!removeTracksFromPlaylist(containerItem.id, selectedIndices, tracksModel.containerUpdateID(), function(result) {
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
        id: tracksModel
        Component.onCompleted: {
            tracksModel.init(Sonos, containerId, false)
            tracksModel.asyncLoad()
        }
    }

    Connections {
        target: tracksModel
        onDataUpdated: {
            tracksModel.asyncLoad()
        }
        onLoaded: {
            tracksModel.resetModel()
        }
        onLoadedMore: {
            if (succeeded)
                tracksModel.appendModel();
        }
        onViewUpdated: {
            if (focusIndex >= 0) {
                if (focusIndex >= tracksModel.totalCount) {
                    focusIndex = tracksModel.totalCount - 1;
                }
                if (focusIndex >= tracksModel.count) {
                    if (!tracksModel.fetchAt(focusIndex)) {
                        trackList.positionViewAtIndex(trackList.count - 1, ListView.End);
                        focusIndex = -1;
                    }
                } else {
                    trackList.positionViewAtIndex(focusIndex, ListView.Center);
                    focusIndex = -1;
                }
            }
        }
    }

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
            Icon {
                height: units.gu(5)
                width: listHeader.childSize
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/images/media-playlist-shuffle.svg"
                label {
                    //: this appears in a button with limited space (around 14 characters)
                    text: qsTr("Shuffle")
                    font.pointSize: units.fs("small")
                    width: listHeader.childSize - units.gu(6)
                    elide: Text.ElideRight
                }
                onClicked: {
                    shuffleModel(tracksModel)
                }
            }
            Icon {
                height: units.gu(5)
                width: listHeader.childSize
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/images/media-playback-start.svg"
                label {
                    //: this appears in a button with limited space (around 14 characters)
                    text: qsTr("Play all")
                    font.pointSize: units.fs("small")
                    width: listHeader.childSize - units.gu(6)
                    elide: Text.ElideRight
                }
                onClicked: {
                    playAll(tracksPage.containerItem)
                }
                enabled: containerItem ? true : false
                visible: enabled
            }
            Icon {
                height: units.gu(5)
                width: listHeader.childSize
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/images/add.svg"
                label {
                    //: this appears in a button with limited space (around 14 characters)
                    text: qsTr("Queue all")
                    font.pointSize: units.fs("small")
                    width: listHeader.childSize - units.gu(6)
                    elide: Text.ElideRight
                }
                onClicked: {
                    addQueue(tracksPage.containerItem)
                }
                enabled: containerItem ? true : false
                visible: enabled
            }
        }

        Column {
            width: parent.width
            height: implicitHeight
            spacing: units.gu(1)
            Label {
               id: titleLabel
               anchors {
                   leftMargin: units.gu(1)
                   rightMargin: units.gu(1)
                   left: parent.left
                   right: parent.right
               }
               color: styleMusic.view.foregroundColor
               elide: Text.ElideRight
               font.pointSize: units.fs("x-large")
               maximumLineCount: 1
               text: containerTitle
               wrapMode: Text.NoWrap
            }

            Label {
               id: summary
               anchors {
                   leftMargin: units.gu(1)
                   rightMargin: units.gu(1)
                   left: parent.left
                   right: parent.right
               }
               color: styleMusic.view.secondaryColor
               elide: Text.ElideRight
               font.pointSize: units.fs("small")
               maximumLineCount: 1
               text: qsTr("%n song(s)", "", tracksModel.totalCount)
               wrapMode: Text.NoWrap
            }

            Item {
                id: spacer
                width: parent.width
                height: units.dp(1)
            }
        }
    }


    Component {
        id: dragDelegate

        SelectMusicListItem {
            id: listItem
            listview: trackList
            reorderable: true
            selectable: true

            onSwipe: {
                focusIndex = index > 0 ? index - 1 : 0;
                delayRemoveSelectedFromPlaylist.selectedIndices = [index]
                delayRemoveSelectedFromPlaylist.start()
                color = "red";
            }

            onReorder: {
                listview.reorder(from, to)
            }

            onClick: {
                var arts = [{art: imageSource}]; // item cover
                dialogSongInfo.open(model, arts,
                                    "qrc:/controls2/Library.qml",
                                    {
                                        "rootPath": "A:ARTIST/" + model.author,
                                        "rootTitle": model.author,
                                        "rootType": LibraryModel.NodePerson,
                                        "isListView": false,
                                        "displayType": LibraryModel.DisplayGrid
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
            rowNumber: model.index + 1
            imageSources: makeCoverSource(model.art, model.author, model.album)
            description: qsTr("Song")

            onImageError: model.art = "" // reset invalid url from model
            onActionPressed: trackClicked(model) // play track
            actionVisible: true
            actionIconSource: "qrc:/images/media-preview-start.svg"
            menuVisible: true

            menuItems: [
                AddToFavorites {
                    isFavorite: listItem.isFavorite
                    description: listItem.description
                    art: model.art
                },
                AddToPlaylist {
                },
                AddToQueue {
                },
                Remove {
                    onTriggered: {
                        focusIndex = index > 0 ? index - 1 : 0;
                        delayRemoveSelectedFromPlaylist.selectedIndices = [index]
                        delayRemoveSelectedFromPlaylist.start()
                        color = "red";
                    }
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
                }
            }

            Component.onCompleted: {
                isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
            }
        }
    }

    MultiSelectListView {
        id: trackList
        anchors.fill: parent
        clip: true
        model: DelegateModel {
            id: visualModel
            model: tracksModel
            delegate: dragDelegate
        }

        signal reorder(int from, int to)

        onReorder: {
            customdebug("Reorder item " + from + " to " + to);
            focusIndex = to;
            if (!reorderTrackInPlaylist(containerItem.id, from, to, tracksModel.containerUpdateID(), function(result) {
                if (!result) {
                    tracksModel.asyncLoad();
                    mainView.actionFailed();
                }
            })) {
                tracksModel.asyncLoad();
                mainView.actionFailed();
            }
        }

        onAtYEndChanged: {
            if (trackList.atYEnd && tracksModel.totalCount > tracksModel.count) {
                tracksModel.asyncLoadMore();
            }
        }

        Connections {
            target: tracksPage
            onSelectAllClicked: {
                trackList.selectAll()
            }
            onSelectNoneClicked: {
                trackList.selectNone()
            }
            onAddToQueueClicked: {
                var indicies = trackList.getSelectedIndices();
                // when select all then add the container if exists
                if (containerItem && indicies.length === tracksModel.count) {
                    if (addQueue(containerItem)) {
                        trackList.selectNone()
                        tracksPage.state = "default"
                    }
                }
                else {
                    var items = [];
                    for (var i = 0; i < indicies.length; i++) {
                        items.push(tracksModel.get(indicies[i]));
                    }
                    if (addMultipleItemsToQueue(items)) {
                        trackList.selectNone()
                        tracksPage.state = "default"
                    }
                }
            }
            onAddToPlaylistClicked: {
                var items = []
                var indicies = trackList.getSelectedIndices();
                // when select all then add the container if exists
                if (containerItem && indicies.length === tracksModel.count)
                    items.push(containerItem);
                else {
                    for (var i = 0; i < indicies.length; i++) {
                        items.push({"payload": tracksModel.get(indicies[i]).payload});
                    }
                }
                stackView.push("qrc:/controls2/AddToPlaylist.qml", {"chosenElements": items})
                trackList.selectNone()
                tracksPage.state = "default"
            }

            onRemoveSelectedClicked: {
                delayRemoveSelectedFromPlaylist.selectedIndices = trackList.getSelectedIndices()
                if (delayRemoveSelectedFromPlaylist.selectedIndices.length > 0)
                    focusIndex = delayRemoveSelectedFromPlaylist.selectedIndices[delayRemoveSelectedFromPlaylist.selectedIndices.length - 1]
                delayRemoveSelectedFromPlaylist.start()
                trackList.selectNone()
                tracksPage.state = "default"
            }
        }

        onHasSelectionChanged: {
            tracksPage.selectAllVisible = !hasSelection
            tracksPage.selectNoneVisible = hasSelection
            tracksPage.addToQueueVisible = hasSelection
            tracksPage.addToPlaylistVisible = hasSelection
            tracksPage.removeSelectedVisible = hasSelection
        }

        Component.onCompleted: {
            tracksPage.selectAllVisible = !hasSelection
            tracksPage.selectNoneVisible = hasSelection
            tracksPage.addToQueueVisible = hasSelection
            tracksPage.addToPlaylistVisible = hasSelection
            tracksPage.removeSelectedVisible = hasSelection
        }
    }

    DialogRemovePlaylist {
        id: dialogRemovePlaylist

        onAccepted: {
            // removing playlist
            removeFromFavorites(tracksPage.containerItem.payload)
            removePlaylist(tracksPage.containerItem.id)
            stackView.pop()
        }
    }

    // Page actions
    optionsMenuVisible: true
    optionsMenuContentItems: [
        MenuItem {
            enabled: containerItem ? true : false
            text: tracksPage.isFavorite ?  qsTr("Remove from favorites") : qsTr("Add to favorites")
            font.pointSize: units.fs("medium")
            onTriggered: {
                if (!tracksPage.isFavorite) {
                    if (addItemToFavorites(containerItem, pageTitle, trackList.headerItem.firstSource))
                        tracksPage.isFavorite = true
                } else {
                    if (removeFromFavorites(containerItem.payload))
                        tracksPage.isFavorite = false
                }
            }
        },
        MenuItem {
            height: (visible ? implicitHeight : 0)
            text: qsTr("Delete")
            font.pointSize: units.fs("medium")
            onTriggered: {
                    dialogRemovePlaylist.open()
            }
        }
    ]

    // check favorite on data loaded
    Connections {
        target: AllFavoritesModel
        onCountChanged: {
            if (containerItem)
                isFavorite = (AllFavoritesModel.findFavorite(containerItem.payload).length > 0)
        }
    }

    Component.onCompleted: {
        if (containerItem)
            isFavorite = (AllFavoritesModel.findFavorite(containerItem.payload).length > 0)
    }
}
