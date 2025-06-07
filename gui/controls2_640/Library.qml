/*
 * Copyright (C) 2022
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
import NosonApp 1.0
import "components"
import "components/Delegates"
import "components/Flickables"
import "components/ListItemActions"
import "components/Dialog"
import "../toolbox.js" as ToolBox

MusicPage {
    id: libraryPage
    objectName: "libraryPage"
    isRoot: mediaModel.isRoot
    multiView: true
    searchable: true

    property string rootPath: ""        // setup the root path of the index to browse
    property string rootTitle: ""       // set up the root title of the index to browse
    property int rootType: 0            // set up the root node type
    property int displayType: 0         // display type for the current page (GRID for root)
    property var nodeItem: null         // handle for global actions (add all, play all)
    property string searchType          // prefered search type

    property int nodeType: 0            // type of the current node
    property bool focusViewIndex: false

    // the model handles search
    property alias searchableModel: mediaModel

    // used to detect view has updated properties since first load.
    // - isFavorite
    property bool taintedView: false

    // internal binding to select the type of view
    // type DisplayUnknown allow switching view list/grid
    property bool showListView: (isListView && displayType === LibraryModel.DisplayUnknown) ||
                displayType === LibraryModel.DisplayTrackList ||
                displayType === LibraryModel.DisplayItemList

    pageTitle: qsTr("My Index")
    pageFlickable: mediaGrid.visible ? mediaGrid : mediaList

    LibraryModel {
      id: mediaModel
    }

    Connections {
        target: mediaModel
        function onDataUpdated() {
            // save current focus
            var focusId;
            if (mediaList.visible) {
                focusId = LibraryModel.firstIndex + mediaList.indexAt(mediaList.contentX, mediaList.contentY);
                mediaList.saveViewFocus(focusId, ListView.Beginning);
            } else if (mediaGrid.visible) {
                focusId = LibraryModel.firstIndex + mediaGrid.indexAt(mediaGrid.contentX, mediaGrid.contentY);
                mediaGrid.saveViewFocus(focusId, GridView.Beginning);
            }
            // refresh data
            mediaModel.asyncLoad();
        }
        function onLoaded(succeeded) {
            if (succeeded) {
                mediaModel.resetModel()
                libraryPage.displayType = mediaModel.displayType() // apply displayType
                libraryPage.nodeType = mediaModel.nodeType() // apply nodeType
                libraryPage.taintedView = false // reset
            } else {
                mediaModel.resetModel();
                emptyState.message = mediaModel.faultString();
                emptyState.active = true;
                customdebug("Fault: " + emptyState.message);
            }
        }
        function onViewUpdated() {
            if (mediaModel.count <= 0) {
                emptyState.message = qsTr("No items found");
                emptyState.active = true;
            } else {
                var idx;
                if (emptyState.active)
                    emptyState.active = false;
                if (focusViewIndex) {
                    // restore parent index position in view
                    idx = mediaModel.viewIndex()
                    focusViewIndex = false;
                    mediaList.positionViewAtIndex(idx, ListView.Center);
                    mediaGrid.positionViewAtIndex(idx, GridView.Center);
                }
            }
        }
        function onPathChanged() {
            var name = mediaModel.pathName();
            if (name === "SEARCH")
                pageTitle = qsTr("Search");
            else if (mediaModel.isRoot)
                pageTitle = rootTitle;
            else
                pageTitle = name;
            // the focus will be resetted for a child page or search
            if (!focusViewIndex) {
                mediaList.saveViewFocus(0, ListView.Beginning);
                mediaGrid.saveViewFocus(0, GridView.Beginning);
            }
        }
    }

    // Overlay to show when no item available
    Loader {
        id: emptyState
        anchors.fill: parent
        active: false
        asynchronous: true
        source: "qrc:/controls2/components/ServiceEmptyState.qml"
        visible: active

        property string message: ""

        onStatusChanged: {
            if (emptyState.status === Loader.Ready)
                item.text = message;
        }
    }

    header: Row {
        id: listHeader
        visible: showListView && (nodeType === LibraryModel.NodeAlbum ||
                                  nodeType === LibraryModel.NodePlaylist ||
                                  nodeType === LibraryModel.NodePlayable)
        height: visible ? units.gu(7) : 0
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
                shuffleModel(mediaModel)
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
                playAll(libraryPage.nodeItem)
            }
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
                addQueue(libraryPage.nodeItem)
            }
            enabled: nodeType !== LibraryModel.NodePlayable
            visible: enabled
        }
    }

    MusicListView {
        id: mediaList
        anchors.fill: parent
        model: mediaModel
        clip: true
        delegate: MusicListItem {
            id: listItem

            property bool held: false
            property string secondaryText: model.description.length > 0 ? model.description
                                  : model.type === LibraryModel.NodeAlbum ? model.artist.length > 0 ? model.artist : qsTr("Album")
                                  : model.type === LibraryModel.NodePerson ? qsTr("Artist")
                                  : model.type === LibraryModel.NodeGenre ? qsTr("Genre")
                                  : model.type === LibraryModel.NodePlaylist ? qsTr("Playlist")
                                  : model.type === LibraryModel.NodeAudioItem && model.canQueue ? model.artist.length > 0 ? model.artist : qsTr("Song")
                                  : model.type === LibraryModel.NodeAudioItem ? qsTr("Radio")
                                  : ""

            onClicked: {
                clickItem(model)
            }
            onPressAndHold: {
                if (model.canPlay) {
                    if (isFavorite && removeFromFavorites(model.payload))
                        isFavorite = false;
                    else if (!isFavorite && addItemToFavorites(model, secondaryText, imageSource))
                        isFavorite = true;
                    libraryPage.taintedView = true;
                }
            }

            color: listItem.held ? "lightgrey" : "transparent"

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                function onLoaded(succeeded) {
                    listItem.isFavorite = model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false
                }
            }

            noCover: model.type === 2 ? "qrc:/images/none.png"
                   : model.canPlay && !model.canQueue ? "qrc:/images/radio.png"
                   : "qrc:/images/no_cover.png"
            imageSources: model.type === LibraryModel.NodeAlbum ? makeCoverSource(model.art, model.artist, model.title)
                        : model.type === LibraryModel.NodePerson ? makeCoverSource(undefined, model.artist, undefined)
                        : model.type === LibraryModel.NodeAudioItem ? makeCoverSource(model.art, model.artist, model.album)
                        : model.type === LibraryModel.NodeGenre ? [{art: "qrc:/images/no_cover.png"}]
                        : model.type === LibraryModel.NodeFolder ? [{art: "qrc:/images/no_cover.png"}]
                        : model.type === LibraryModel.NodePlaylist ? [{art: "qrc:/images/no_cover.png"}]
                        : model.type === LibraryModel.NodePlayable ? [{art: "qrc:/images/no_cover.png"}]
                        : model.canPlay && !model.canQueue ? [{art: "qrc:/images/streaming.png"}]
                        : model.art !== "" ? [{art: model.art}]
                        : [{art: "qrc:/images/no_cover.png"}]
            description: model.description.length > 0 ? model.description
                    : model.type === LibraryModel.NodeAlbum ? model.artist.length > 0 ? model.artist : qsTr("Album")
                    : model.type === LibraryModel.NodePerson ? qsTr("Artist")
                    : model.type === LibraryModel.NodeGenre ? qsTr("Genre")
                    : model.type === LibraryModel.NodePlaylist ? qsTr("Playlist")
                    : model.type === LibraryModel.NodeAudioItem && model.canQueue ? model.artist.length > 0 ? model.artist : qsTr("Song")
                    : model.type === LibraryModel.NodeAudioItem ? qsTr("Radio")
                    : ""
            onActionPressed: { if (model.canQueue) playItem(model); }
            actionVisible: model.canQueue || isFavorite
            actionIconSource: "qrc:/images/media-preview-start.svg"
            menuVisible: model.canPlay || model.canQueue

            menuItems: [
                AddToFavorites {
                    isFavorite: listItem.isFavorite
                    enabled: model.canPlay
                    visible: enabled
                    description: listItem.description
                    art: model.art

                    onTriggered: {
                        libraryPage.taintedView = true;
                    }
                },
                AddToPlaylist {
                    enabled: model.canQueue
                    visible: enabled
                },
                AddToQueue {
                    enabled: model.canQueue
                    visible: enabled
                }
            ]

            coverSize: units.gu(5)

            column: Column {
                Label {
                    id: mediaTitle
                    color: styleMusic.view.primaryColor
                    font.pointSize: units.fs("medium")
                    objectName: "itemtitle"
                    text: model.title
                }

                Label {
                    id: mediaDescription
                    color: styleMusic.view.secondaryColor
                    font.pointSize: units.fs("x-small")
                    text: listItem.description
                    visible: text !== ""
                }
            }

            Component.onCompleted: {
                listItem.isFavorite = model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false
            }
        }

        opacity: showListView ? 1.0 : 0.0
        visible: opacity > 0.0

        property bool fetchEnabled: false // property to enable/disable fetch on move

        function focusView(focusId, focusMode) {
            var index = (focusId < mediaModel.firstIndex ? 0 : focusId - mediaModel.firstIndex);
            positionViewAtIndex(index, focusMode);
            // finally enable fetch on move
            fetchEnabled = true;
        }

        function saveViewFocus(focusId, focusMode) {
            // disable fetch on move
            fetchEnabled = false;
            ToolBox.connectOnce(mediaModel.onViewUpdated, function(){
                mediaList.focusView(focusId, focusMode);
            });
        }

        onAtYEndChanged: {
            if (visible && fetchEnabled && mediaList.atYEnd &&
                    mediaModel.totalCount > (mediaModel.firstIndex + mediaModel.count)) {
                if (mediaModel.fetchBack()) {
                    var focusId = mediaModel.firstIndex + mediaModel.count;
                    mediaList.saveViewFocus(focusId, ListView.End);
                }
            }
        }
        onAtYBeginningChanged: {
            if (visible && fetchEnabled && mediaList.atYBeginning &&
                    mediaModel.firstIndex > 0) {
                if (mediaModel.fetchFront()) {
                    var focusId = mediaModel.firstIndex - 1;
                    mediaList.saveViewFocus(focusId, ListView.Beginning);
                }
            }
        }
    }

    MusicGridView {
        id: mediaGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9)

        model: mediaModel

        delegate: Card {
            id: mediaCard
            height: mediaGrid.cellHeight
            width: mediaGrid.cellWidth
            primaryText: model.title
            secondaryText: model.description.length > 0 ? model.description
                         : model.type === LibraryModel.NodeAlbum ? model.artist.length > 0 ? model.artist : qsTr("Album")
                         : model.type === LibraryModel.NodePerson ? qsTr("Artist")
                         : model.type === LibraryModel.NodeGenre ? qsTr("Genre")
                         : model.type === LibraryModel.NodePlaylist ? qsTr("Playlist")
                         : model.type === LibraryModel.NodeAudioItem && model.canQueue ? model.artist.length > 0 ? model.artist : qsTr("Song")
                         : model.type === LibraryModel.NodeAudioItem ? qsTr("Radio")
                         : ""

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                function onLoaded(succeeded) {
                    isFavorite = model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false
                }
            }

            canPlay: model.canPlay

            overlay: false // item icon could be transparent
            noCover: model.type === 2 ? "qrc:/images/none.png"
                   : "qrc:/images/no_cover.png"
            coverSources: model.type === LibraryModel.NodeAlbum ? makeCoverSource(model.art, model.artist, model.title)
                        : model.type === LibraryModel.NodePerson ? makeCoverSource(undefined, model.artist, undefined)
                        : model.type === LibraryModel.NodeAudioItem ? makeCoverSource(model.art, model.artist, model.album)
                        : model.type === LibraryModel.NodeGenre ? [{art: "qrc:/images/no_cover.png"}]
                        : model.type === LibraryModel.NodeFolder ? [{art: "qrc:/images/no_cover.png"}]
                        : model.type === LibraryModel.NodePlaylist ? [{art: "qrc:/images/no_cover.png"}]
                        : model.type === LibraryModel.NodePlayable ? [{art: "qrc:/images/no_cover.png"}]
                        : model.canPlay && !model.canQueue ? [{art: "qrc:/images/streaming.png"}]
                        : model.art !== "" ? [{art: model.art}]
                        : [{art: "qrc:/images/no_cover.png"}]

            onClicked: clickItem(model)
            onPressAndHold: {
                if (model.canPlay) {
                    if (isFavorite && removeFromFavorites(model.payload))
                        isFavorite = false;
                    else if (!isFavorite && addItemToFavorites(model, secondaryText, imageSource))
                        isFavorite = true;
                    libraryPage.taintedView = true;
                }
            }
            onPlayClicked: { if (model.canPlay) playItem(model); }

            Component.onCompleted: {
                mediaCard.isFavorite = model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false
            }
        }

        opacity: showListView ? 0.0 : 1.0
        visible: opacity > 0.0

        property bool fetchEnabled: false // property to enable/disable fetch on move

        function focusView(focusId, focusMode) {
            var index = (focusId < mediaModel.firstIndex ? 0 : focusId - mediaModel.firstIndex);
            positionViewAtIndex(index, focusMode);
            // finally enable fetch on move
            fetchEnabled = true;
        }

        function saveViewFocus(focusId, focusMode) {
            // disable fetch on move
            fetchEnabled = false;
            ToolBox.connectOnce(mediaModel.onViewUpdated, function(){
                mediaGrid.focusView(focusId, focusMode);
            });
        }

        onAtYEndChanged: {
            if (visible && fetchEnabled && mediaGrid.atYEnd &&
                    mediaModel.totalCount > (mediaModel.firstIndex + mediaModel.count)) {
                if (mediaModel.fetchBack()) {
                    var focusId = mediaModel.firstIndex + mediaModel.count;
                    mediaGrid.saveViewFocus(focusId, GridView.End);
                }
            }
        }
        onAtYBeginningChanged: {
            if (visible && fetchEnabled && mediaGrid.atYBeginning &&
                    mediaModel.firstIndex > 0) {
                if (mediaModel.fetchFront()) {
                    var focusId = mediaModel.firstIndex - 1;
                    mediaGrid.saveViewFocus(focusId, GridView.Beginning);
                }
            }
        }
    }

    Component.onCompleted: {
        // on first load enable fetching by focusing views
        mediaList.saveViewFocus(0,ListView.Beginning);
        mediaGrid.saveViewFocus(0,GridView.Beginning);
        // init and setup the root node with provided properties
        mediaModel.init(Sonos, rootPath, false, displayType, rootType)
        if (rootPath.length === 0) {
            // no root path: open the search dialog ...
            // if the dialog is rejected then pop this page
            ToolBox.connectOnce(dialogSearch.closed, function(){
                // fix qt5.9
                if (!dialogSearch._search)
                    stackView.pop();
                //if (dialogSearch.result === Dialog.Rejected)
                //    stackView.pop();
            });
            dialogSearch.open();
        } else {
            mediaModel.asyncLoad()
            searchable = (mediaModel.listSearchCategories().length > 0)
        }
        if (settings.preferListView)
            isListView = true
    }

    onListViewClicked: {
        settings.preferListView = isListView
    }

    onGoUpClicked: {
        focusViewIndex = true;
        mediaModel.asyncLoadParent();
    }

    function clickItem(model) {
        if (model.isContainer) {
            libraryPage.nodeItem = makeContainerItem(model);
            mediaModel.asyncLoadChild(model.id, model.title, model.displayType, model.type, model.index);
        } else {
            var songModel = {
                "id": model.id,
                "payload": model.payload,
                "title": model.title,
                "author": model.artist,
                "album": model.album,
                "description": model.description,
                "albumTrackNo": model.albumTrackNo
            };
            dialogSongInfo.open(songModel, model.art,
                                "", undefined, false,
                                model.canPlay,
                                model.canQueue,
                                model.isContainer
                                );
        }
    }

    function playItem(model) {
        if (model.canPlay) {
            if (model.canQueue) {
                if (model.isContainer)
                    playAll(model);
                else
                    trackClicked(model);
            } else {
                radioClicked(model);
            }
        }
    }

    DialogSearchMusic {
        id: dialogSearch
        searchableModel: mediaModel
        searchType: libraryPage.searchType
    }

    onSearchClicked: dialogSearch.open();
}
