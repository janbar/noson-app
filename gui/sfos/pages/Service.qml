/*
 * Copyright (C) 2017
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
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/ListItemActions"
import "../components/Dialog"


MusicPage {
    id: servicePage
    objectName: "servicePage"
    isRoot: mediaModel.isRoot
    multiView: true
    searchable: true

    property var serviceItem: null
    property int displayType: 3  // display type for root
    property int parentDisplayType: 0
    property bool focusViewIndex: false

    // the model handles search
    property alias searchableModel: mediaModel

    // used to detect view has updated properties since first load.
    // - isFavorite
    property bool taintedView: false

    pageTitle: serviceItem.title
    pageFlickable: mediaGrid.visible ? mediaGrid : mediaList

    MediaModel {
      id: mediaModel
    }

    function restoreFocusViewIndex() {
        var idx = mediaModel.viewIndex()
        if (mediaModel.count <= idx) {
          mediaModel.asyncLoadMore() // load more !!!
        } else {
            focusViewIndex = false;
            mediaList.positionViewAtIndex(idx, ListView.Center);
            mediaGrid.positionViewAtIndex(idx, GridView.Center);
        }
    }

    Connections {
        target: mediaModel
        onDataUpdated: mediaModel.asyncLoad()
        onLoaded: {
            if (succeeded) {
                mediaModel.resetModel()
                servicePage.displayType = servicePage.parentDisplayType // apply displayType
                servicePage.taintedView = false // reset
                if (focusViewIndex) {
                    // restore index position in view
                    restoreFocusViewIndex()
                } else {
                    mediaList.positionViewAtIndex(0, ListView.Top);
                    mediaGrid.positionViewAtIndex(0, GridView.Top);
                }

                if (mediaModel.count > 0) {
                    if (emptyState.active)
                        emptyState.active = false;
                } else {
                    emptyState.message = qsTr("No items found");
                    emptyState.active = true;
                }

            } else {
                // don't show registration fault
                if (!mediaModel.isAuthExpired) {
                    mediaModel.resetModel();
                    emptyState.message = mediaModel.faultString();
                    emptyState.active = true;
                    customdebug("Fault: " + emptyState.message);
                }
            }
        }
        onLoadedMore: {
            if (succeeded) {
                mediaModel.appendModel()
                if (focusViewIndex) {
                    // restore index position in view
                    restoreFocusViewIndex()
                }
            } else if (focusViewIndex) {
                focusViewIndex = false;
                mediaList.positionViewAtEnd();
                mediaGrid.positionViewAtEnd();
            }
        }

        onPathChanged: {
            if (mediaModel.isRoot) {
                pageTitle = serviceItem.title;
            } else {
                var name = mediaModel.pathName();
                if (name === "SEARCH")
                    pageTitle = serviceItem.title + " : " + qsTr("Search");
                else
                    pageTitle = serviceItem.title + " : " + name;
            }
        }
    }

    onDisplayTypeChanged: {
        isListView = (displayType === 0 /*Grid*/ || displayType === 3 /*Editorial*/) ? false : true
    }

    // Overlay to show when no item available
    Loader {
        id: emptyState
        anchors.fill: parent
        active: false
        asynchronous: true
        source: "qrc:/sfos/components/ServiceEmptyState.qml"
        visible: active

        property string message: ""

        onStatusChanged: {
            if (emptyState.status === Loader.Ready)
                item.text = message;
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
            onPressAndHold: held = true
            onReleased: held = false
            onClicked: {
                if (model.isContainer)
                    clickItem(model)
            }

            color: listItem.held ? "lightgrey" : "transparent"

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onCountChanged: {
                    listItem.isFavorite = model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false
                }
            }

            noCover: model.type === 2 ? "qrc:/images/none.png"
                   : model.canPlay && !model.canQueue ? "qrc:/images/radio.png"
                   : "qrc:/images/no_cover.png"
            imageSources: model.art !== "" ? [{art: model.art}]
                        : model.type === 2 ? [{art: "qrc:/images/none.png"}]
                        : model.canPlay && !model.canQueue ? [{art: "qrc:/images/radio.png"}]
                        : [{art: "qrc:/images/no_cover.png"}]
            description: model.description.length > 0 ? model.description
                    : model.type === 1 ? model.artist.length > 0 ? model.artist : qsTr("Album")
                    : model.type === 2 ? qsTr("Artist")
                    : model.type === 3 ? qsTr("Genre")
                    : model.type === 4 ? qsTr("Playlist")
                    : model.type === 5 && model.canQueue ? model.artist.length > 0 ? model.artist : qsTr("Song")
                    : model.type === 5 ? qsTr("Radio")
                    : ""
            onActionPressed: playItem(model)
            actionVisible: model.canPlay
            actionIconSource: "qrc:/images/media-preview-start.svg"
            menuVisible: model.canPlay || model.canQueue

            menu: ContextMenu {
                AddToFavorites {
                    isFavorite: listItem.isFavorite
                    enabled: model.canPlay
                    visible: enabled
                    description: listItem.description
                    art: model.art

                    onDelayedClick: {
                        servicePage.taintedView = true;
                    }
                }
                AddToPlaylist {
                    enabled: model.canQueue
                    visible: enabled
                }
                AddToQueue {
                    enabled: model.canQueue
                    visible: enabled
                }
            }

            coverSize: units.gu(5)

            column: Column {
                Label {
                    id: mediaTitle
                    color: styleMusic.view.primaryColor
                    font.pixelSize: units.fx("medium")
                    objectName: "itemtitle"
                    text: model.title
                }

                Label {
                    id: mediaDescription
                    color: styleMusic.view.secondaryColor
                    font.pixelSize: units.fx("x-small")
                    text: listItem.description
                    visible: text !== ""
                }
            }

            Component.onCompleted: {
                isFavorite = model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false
            }
        }

        opacity: isListView ? 1.0 : 0.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }

        onAtYEndChanged: {
            if (mediaList.atYEnd && mediaModel.totalCount > mediaModel.count) {
                mediaModel.asyncLoadMore()
            }
        }
    }

    MusicGridView {
        id: mediaGrid
        itemWidth: displayType == 3 /*Editorial*/ ? units.gu(12) : units.gu(15)
        heightOffset: units.gu(9)
        clip: true
        model: mediaModel

        delegate: Card {
            id: favoriteCard
            primaryText: model.title
            secondaryText: model.description.length > 0 ? model.description
                         : model.type === 1 ? model.artist.length > 0 ? model.artist : qsTr("Album")
                         : model.type === 2 ? qsTr("Artist")
                         : model.type === 3 ? qsTr("Genre")
                         : model.type === 4 ? qsTr("Playlist")
                         : model.type === 5 && model.canQueue ? model.artist.length > 0 ? model.artist : qsTr("Song")
                         : model.type === 5 ? qsTr("Radio")
                         : ""

            isFavorite: model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onLoaded: {
                    isFavorite = model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false
                }
            }

            canPlay: model.canPlay

            overlay: false // item icon could be transparent
            noCover: model.type === 2 ? "qrc:/images/none.png"
                   : model.canPlay && !model.canQueue ? "qrc:/images/radio.png"
                   : "qrc:/images/no_cover.png"
            coverSources: model.art !== "" ? [{art: model.art}]
                        : model.type === 2 ? [{art: "qrc:/images/none.png"}]
                        : model.canPlay && !model.canQueue ? [{art: "qrc:/images/radio.png"}]
                        : [{art: "qrc:/images/no_cover.png"}]

            onClicked: clickItem(model)
            onPressAndHold: {
                if (model.canPlay) {
                    if (isFavorite && removeFromFavorites(model.payload))
                        isFavorite = false;
                    else if (!isFavorite && addItemToFavorites(model, secondaryText, imageSource))
                        isFavorite = true;
                    servicePage.taintedView = true;
                } else {
                    servicePage.isListView = true
                }
            }
            onPlayClicked: playItem(model)
        }

        opacity: isListView ? 0.0 : 1.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }

        onAtYEndChanged: {
            if (mediaGrid.atYEnd && mediaModel.totalCount > mediaModel.count) {
                mediaModel.asyncLoadMore()
            }
        }
    }

    Component {
        id: menuItemComp
        MenuItem {
        }
    }

    property MenuItem menuItemGoBack: null
    onIsRootChanged: menuItemGoBack.visible = !isRoot // enable the menu when the content isn't root

    Component.onCompleted: {
        mediaModel.init(Sonos, serviceItem.payload, false)
        mediaModel.asyncLoad()
        // create the menu item to navigate back. Starting from root it isn't visible
        menuItemGoBack = menuItemComp.createObject(pageMenu, {"text" : qsTr("Go back"), "visible" : false})
        menuItemGoBack.onClicked.connect(goUpClicked)
    }

    onGoUpClicked: {
        // change view depending of parent display type
        servicePage.parentDisplayType = mediaModel.parentDisplayType();
        focusViewIndex = true;
        mediaModel.asyncLoadParent();
    }

    function clickItem(model) {
        if (model.isContainer) {
            servicePage.parentDisplayType = model.displayType;
            mediaModel.asyncLoadChild(model.id, model.title, servicePage.displayType, model.index);
        } else {
            var songModel = {
                "id": model.id,
                "payload": model.payload,
                "title": model.title,
                "author": model.artist,
                "album": model.album,
                "description": model.description
            };
            dialogSongInfo.open(songModel, [{art: model.art}], model.canPlay && model.canQueue, false); // show actions
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

    onSearchClicked: {
        dialogSearch.searchableModel = searchableModel
        dialogSearch.open();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Service registration
    ////

    Loader {
        id: registeringService
        anchors.fill: parent
        source: "qrc:/sfos/components/ServiceRegistration.qml"
        active: false
    }

    Loader {
        id: loginService
        anchors.fill: parent
        source: "qrc:/sfos/components/ServiceLogin.qml"
        active: false
    }

    Connections {
        target: mediaModel
        onIsAuthExpiredChanged: {
            var auth;
            if (mediaModel.isAuthExpired) {
                if (mediaModel.policyAuth == 1) {
                    if (!loginService.active) {
                        // first try with saved login/password
                        auth = mediaModel.getDeviceAuth();
                        if (auth['key'].length === 0 || mediaModel.requestSessionId(auth['username'], auth['key']) === 0)
                            loginService.active = true; // show login registration
                        else {
                            // refresh the model
                            mediaModel.asyncLoad();
                        }
                    }
                } else if (mediaModel.policyAuth == 2 || mediaModel.policyAuth == 3) {
                    if (registeringService.active)
                        registeringService.active = false; // restart new registration
                    else
                        mediaModel.clearData();
                    registeringService.active = true;
                }
            } else {
                loginService.active = false;
                registeringService.active = false;
                mainView.jobRunning = true;
                // save new incarnation of accounts settings
                auth = mediaModel.getDeviceAuth();
                var acls = deserializeACLS(settings.accounts);
                var _acls = [];
                for (var i = 0; i < acls.length; ++i) {
                    if (acls[i].type === auth['type'] && acls[i].sn === auth['serialNum'])
                        continue;
                    else
                        _acls.push(acls[i]);
                }
                _acls.push({type: auth['type'], sn: auth['serialNum'], key: auth['key'], token: auth['token'], username: auth['username']});
                settings.accounts = serializeACLS(_acls);
                // refresh the model
                mediaModel.asyncLoad();
            }
        }
    }
}
