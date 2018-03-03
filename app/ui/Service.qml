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

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import Ubuntu.Thumbnailer 0.1
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"
import "../components/ListItemActions"


MusicPage {
    id: servicePage
    objectName: "servicePage"

    property bool isListView: false
    property var serviceItem: null
    property bool loaded: false  // used to detect difference between first and further loads
    property bool isRoot: mediaModel.isRoot
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
    searchable: true
    searchResultsCount: mediaModel.count
    state: "default"
    states: [
        ServiceHeadState {
            thisPage: servicePage
            searchEnabled: true
            thisHeader {
                extension: DefaultSections { }
            }
        },
        MultiSelectHeadState {
            listview: mediaList
            thisPage: servicePage
            addToQueue: false
            addToPlaylist: false
            removable: false
            thisHeader {
                extension: DefaultSections { }
            }
        }
    ]

    width: mainPageStack.width

    BlurredBackground {
            id: blurredBackground
            height: parent.height
            art: serviceItem.id === "SA_RINCON65031_0" ? Qt.resolvedUrl("../graphics/tunein.png") : serviceItem.icon
    }

    property alias model: mediaModel // used in ServiceHeadState

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
                    pageTitle = serviceItem.title + " : " + i18n.tr("Search");
                else
                    pageTitle = serviceItem.title + " : " + name;
            }
        }
    }

    onDisplayTypeChanged: {
        isListView = (displayType === 0 /*Grid*/ || displayType === 3 /*Editorial*/) ? false : true
    }

    // Hack for autopilot otherwise Albums appears as MusicPage
    // due to bug 1341671 it is required that there is a property so that
    // qml doesn't optimise using the parent type
    property bool bug1341671workaround: true

    MultiSelectListView {
        id: mediaList
        anchors {
            bottomMargin: units.gu(2)
            fill: parent
            topMargin: units.gu(2)
        }
        model: mediaModel

        onStateChanged: {
            if (state === "multiselectable") {
                servicePage.state = "selection"
            } else {
                servicePage.state = "default"
            }
        }

        delegate: MusicListItem {
            id: mediaItem
            color: "transparent"
            property string description: model.description.length > 0 ? model.description
                                       : model.type === 1 ? model.artist.length > 0 ? model.artist : i18n.tr("Album")
                                       : model.type === 2 ? i18n.tr("Artist")
                                       : model.type === 3 ? i18n.tr("Genre")
                                       : model.type === 4 ? i18n.tr("Playlist")
                                       : model.type === 5 && model.canQueue ? model.artist.length > 0 ? model.artist : i18n.tr("Song")
                                       : model.type === 5 ? i18n.tr("Radio")
                                       : ""
            column: Column {
                Label {
                    id: mediaTitle
                    color: styleMusic.common.music
                    fontSize: "medium"
                    objectName: "itemtitle"
                    text: model.title
                }

                Label {
                    id: mediaDescription
                    color: styleMusic.common.subtitle
                    fontSize: "small"
                    text: description
                }
            }
            leadingActions: ListItemActions {
                actions: [
                ]
            }

            height: units.gu(7)

            noCover: model.type === 2 ? Qt.resolvedUrl("../graphics/none.png")
                   : model.canPlay && !model.canQueue ? Qt.resolvedUrl("../graphics/radio.png")
                   : Qt.resolvedUrl("../graphics/no_cover.png")

            imageSource: model.art !== undefined && model.art.length > 0 ? model.art
                       : model.type === 2 ? Qt.resolvedUrl("../graphics/none.png")
                       : model.canPlay && !model.canQueue ? Qt.resolvedUrl("../graphics/radio.png")
                       : Qt.resolvedUrl("../graphics/no_cover.png")

            multiselectable: false

            trailingActions: ListItemActions {
                actions: [
                    Action {
                        iconName: "media-playback-start"
                        objectName: "playAction"
                        text: i18n.tr("Play")
                        visible: model.canPlay
                        onTriggered: playItem(model)
                    },
                    Action {
                        iconName: "add"
                        objectName: "addToQueueAction"
                        text: i18n.tr("Add to queue")
                        visible: model.canQueue
                        onTriggered: addQueue({id: model.Id, payload: model.payload})
                    },
                    Action {
                        iconName: "add-to-playlist"
                        objectName: "addToPlaylistAction"
                        text: i18n.tr("Add to playlist")
                        visible: model.canQueue
                        onTriggered: {
                            mainPageStack.push(Qt.resolvedUrl("AddToPlaylist.qml"),
                                               {"chosenElements": [{id: model.Id, payload: model.payload}]})
                        }
                    },
                    Action {
                        property bool isFavorite: false

                        iconName: isFavorite ? "starred" : "scope-manager"
                        objectName: "ActionFavorite"
                        text: i18n.tr("Favorite")
                        visible: model.canPlay

                        Component.onCompleted: {
                            isFavorite = model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false
                        }

                        onTriggered: {
                            if (isFavorite && removeFromFavorites(model.payload))
                                isFavorite = false;
                            else if (!isFavorite && addItemToFavorites(model, mediaItem.description, mediaItem.imageSource))
                                isFavorite = true;
                            servicePage.taintedView = true;
                        }
                    }
                ]
                delegate: ActionDelegate {
                }
            }

            onItemClicked: clickItem(model)
        }

        visible: isListView ? true : false

        onAtYEndChanged: {
            if (mediaList.atYEnd && mediaModel.totalCount > mediaModel.count) {
                focusViewIndex = true;
                mediaModel.asyncLoadMore()
            }
        }
    }

    MusicGridView {
        id: mediaGrid
        itemWidth: displayType == 3 /*Editorial*/ ? units.gu(12) : units.gu(15)
        heightOffset: units.gu(9.5)

        model: mediaModel

        onStateChanged: {
            if (state === "multiselectable") {
                servicePage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                servicePage.state = "default"
            }
        }

        delegate: Card {
            id: favoriteCard
            primaryText: model.title
            secondaryText: model.description !== undefined && model.description.length > 0 ? model.description
                         : model.type === 1 ? model.artist.length > 0 ? model.artist : i18n.tr("Album")
                         : model.type === 2 ? i18n.tr("Artist")
                         : model.type === 3 ? i18n.tr("Genre")
                         : model.type === 4 ? i18n.tr("Playlist")
                         : model.type === 5 && model.canQueue ? model.artist.length > 0 ? model.artist : i18n.tr("Song")
                         : model.type === 5 ? i18n.tr("Radio")
                         : ""
            isFavorite: model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false
            canPlay: model.isContainer && model.canPlay ? true : false

            noCover: model.type === 2 ? Qt.resolvedUrl("../graphics/none.png")
                   : model.canPlay && !model.canQueue ? Qt.resolvedUrl("../graphics/radio.png")
                   : Qt.resolvedUrl("../graphics/no_cover.png")

            coverSources: model.art !== "" ? [{art: model.art}]
                        : model.type === 2 ? [{art: Qt.resolvedUrl("../graphics/none.png")}]
                        : model.canPlay && !model.canQueue ? [{art: Qt.resolvedUrl("../graphics/radio.png")}]
                        : [{art: Qt.resolvedUrl("../graphics/no_cover.png")}]

            onClicked: clickItem(model)

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onCountChanged: {
                    isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

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

        visible: isListView ? false : true

        onAtYEndChanged: {
            if (mediaGrid.atYEnd && mediaModel.totalCount > mediaModel.count) {
                mediaModel.asyncLoadMore()
            }
        }
    }

    Component.onCompleted: {
        mediaModel.init(Sonos, serviceItem.payload, false)
        mediaModel.asyncLoad()
    }

    function goUp() {
        // change view depending of parent display type
        servicePage.parentDisplayType = mediaModel.parentDisplayType();
        focusViewIndex = true;
        mediaModel.asyncLoadParent();
    }

    function clickItem(model) {
        if (model.isContainer) {
            servicePage.parentDisplayType = model.displayType;
            mediaModel.asyncLoadChild(model.id, model.title, servicePage.displayType, model.index);
        } else if (model.canPlay) {
            if (model.canQueue)
                trackClicked(model);
            else
                radioClicked(model);
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

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Service registration
    ////

    Loader {
        id: registeringService
        anchors.fill: parent
        source: "../components/ServiceRegistration.qml"
        active: false
    }

    Loader {
        id: loginService
        anchors.fill: parent
        source: "../components/ServiceLogin.qml"
        active: false
    }

    Connections {
        target: mediaModel
        onIsAuthExpiredChanged: {
            if (mediaModel.isAuthExpired) {
                if (mediaModel.policyAuth == 1) {
                    if (!loginService.active) {
                        // first try with saved login/password
                        var auth = mediaModel.getDeviceAuth();
                        if (auth.key.length === 0 || mediaModel.requestSessionId(auth.username, auth.key) === 0)
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
                var auth = mediaModel.getDeviceAuth();
                var acls = deserializeACLS(startupSettings.accounts);
                var _acls = [];
                for (var i = 0; i < acls.length; ++i) {
                    if (acls[i].type === auth.type && acls[i].sn === auth.serialNum)
                        continue;
                    else
                        _acls.push(acls[i]);
                }
                _acls.push({type: auth.type, sn: auth.serialNum, key: auth.key, token: auth.token, username: auth.username});
                startupSettings.accounts = serializeACLS(_acls);
                // refresh the model
                mediaModel.asyncLoad();
            }
        }
    }
}
