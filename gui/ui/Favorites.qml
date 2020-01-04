/*
 * Copyright (C) 2016, 2017
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
import QtQuick.Layouts 1.3
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/ListItemActions"


MusicPage {
    id: favoritesPage
    objectName: "favoritesPage"
    pageTitle: qsTr("Favorites")
    pageFlickable: favoriteGrid.visible ? favoriteGrid : favoriteList
    multiView: true
    searchable: true

    onSearchClicked: filter.visible = true

    header: MusicFilter {
        id: filter
        visible: false
    }

    SortFilterModel {
        id: favoritesModelFilter
        model: AllFavoritesModel
        sort.property: "title"
        sort.order: Qt.AscendingOrder
        sortCaseSensitivity: Qt.CaseInsensitive
        filter.property: "normalized"
        filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
        filterCaseSensitivity: Qt.CaseInsensitive
    }

    MusicListView {
        id: favoriteList
        anchors.fill: parent
        model: favoritesModelFilter
        delegate: MusicListItem {
            id: listItem

            onSwipe: {
                favoriteList.focusIndex = model.index > 0 ? model.index - 1 : 0;
                delayRemoveFavorite.start();
                color = "red";
            }

            property bool held: false
            onPressAndHold: held = true
            onReleased: held = false

            color: listItem.held ? "lightgrey" : "transparent"

            noCover: model.type === 5 && !model.canQueue ? "qrc:/images/radio.png"
                   : model.type === 2 ? "qrc:/images/none.png"
                   : "qrc:/images/no_cover.png"
            imageSources: model.type === 1 ? makeCoverSource(model.art, model.artist, model.title)
                        : model.type === 2 ? makeCoverSource(undefined, model.artist, undefined)
                        : model.type === 5 && model.canQueue ? makeCoverSource(model.art, model.author, model.album)
                        : model.type === 5 && model.art === "" ? "qrc:/images/radio.png"
                        : makeCoverSource(model.art, undefined, undefined)
            description:  model.description.length > 0 ? model.description
                       : model.type === 1 ? qsTr("Album")
                       : model.type === 2 ? qsTr("Artist")
                       : model.type === 3 ? qsTr("Genre")
                       : model.type === 4 ? qsTr("Playlist")
                       : model.type === 5 ? qsTr("Song")
                       : "";

            onImageError: model.art = "" // reset invalid url from model
            onActionPressed: clickItem(model)
            actionVisible: true
            actionIconSource: "qrc:/images/media-preview-start.svg"
            menuVisible: true

            menuItems: [
                AddToQueue {
                    enabled: model.canQueue
                    visible: enabled
                },
                AddToPlaylist {
                    enabled: model.canQueue
                    visible: enabled
                },
                Remove {
                    onTriggered: {
                        favoriteList.focusIndex = model.index > 0 ? model.index - 1 : 0;
                        delayRemoveFavorite.start();
                        color: "red";
                    }
                }
            ]

            coverSize: units.gu(5)

            column: Column {
                Label {
                    id: favoriteTitle
                    color: styleMusic.view.primaryColor
                    font.pointSize: units.fs("medium")
                    text: model.title
                }

                Label {
                    id: favoriteDescription
                    color: styleMusic.view.secondaryColor
                    font.pointSize: units.fs("x-small")
                    text: listItem.description
                }
            }

            Timer {
                id: delayRemoveFavorite
                interval: 100
                onTriggered: {
                    var future = Sonos.tryDestroyFavorite(model.id);
                    future.finished.connect(mainView.actionFinished);
                    future.start();
                }
            }
        }

        property int focusIndex: 0

        Connections {
            target: AllFavoritesModel
            onLoaded: {
                if (favoriteList.focusIndex > 0) {
                    favoriteList.positionViewAtIndex(favoriteList.focusIndex, ListView.Center);
                    favoriteList.focusIndex = 0;
                }
            }
        }

        opacity: isListView ? 1.0 : 0.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

    MusicGridView {
        id: favoriteGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9)

        model: favoritesModelFilter

        delegate: Card {
            id: favoriteCard
            primaryText: model.title
            secondaryText: model.description.length > 0 ? model.description
                         : model.type === 1 ? qsTr("Album")
                         : model.type === 2 ? qsTr("Artist")
                         : model.type === 3 ? qsTr("Genre")
                         : model.type === 4 ? qsTr("Playlist")
                         : model.type === 5 ? qsTr("Song")
                         : ""
            overlay: false // item icon could be transparent
            noCover: model.type === 5 && !model.canQueue ? "qrc:/images/radio.png"
                   : model.type === 2 ? "qrc:/images/none.png"
                   : "qrc:/images/no_cover.png"
            coverSources: model.type === 1 ? makeCoverSource(model.art, model.artist, model.title)
                        : model.type === 2 ? makeCoverSource(undefined, model.artist, undefined)
                        : model.type === 5 && model.canQueue ? makeCoverSource(model.art, model.author, model.album)
                        : model.type === 5 && model.art === "" ? [{art: "qrc:/images/radio.png"}]
                        : makeCoverSource(model.art, undefined, undefined)

            onImageError: model.art = "" // reset invalid url from model
            onClicked: clickItem(model)
        }

        opacity: isListView ? 0.0 : 1.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

    Timer {
        id: delayFavoritePlayAll
        interval: 100
        property QtObject model
        onTriggered: {
            // clear queue when playing bundle
            if (model.type !== 5 && model.canQueue) {
                player.removeAllTracksFromQueue(function(result) {
                    if (result) {
                        player.playFavorite(model, mainView.actionFinished);
                    } else {
                        mainView.actionFailed();
                    }
                });
            } else {
                player.playFavorite(model, mainView.actionFinished);
            }
        }
    }

    function clickItem(model) {
        if (!model.isService) {
            if (model.type === 1) {
                stackView.push("qrc:/ui/SongsView.qml",
                                   {
                                       "containerItem": {id: model.objectId, payload: model.object},
                                       "songSearch": model.objectId,
                                       "album": model.album,
                                       "artist": model.artist,
                                       "covers": [{art: model.art}],
                                       "isAlbum": true,
                                       "genre": "",
                                       "pageTitle": qsTr("Album"),
                                       "line1": model.artist !== undefined ? model.artist : "",
                                       "line2": model.album !== undefined ? model.album : qsTr("Unknown Album")
                                   })
            }
            else if (model.type === 2) {
                stackView.push("qrc:/ui/ArtistView.qml",
                                   {
                                       "containerItem": {id: model.objectId, payload: model.object},
                                       "artistSearch": model.objectId,
                                       "artist": model.artist,
                                       "covers": makeCoverSource(undefined, model.artist, undefined),
                                       "pageTitle": qsTr("Artist")
                                   })
            }
            else if (model.type === 3) {
                stackView.push("qrc:/ui/SongsView.qml",
                                   {
                                       "containerItem": {id: model.objectId, payload: model.object},
                                       "songSearch": model.objectId + "//",
                                       "covers": [],
                                       "album": "",
                                       "genre": model.title,
                                       "pageTitle": qsTr("Genre"),
                                       "line1": "",
                                       "line2": model.title
                                   })
            }
            else if (model.type === 4) {
                stackView.push("qrc:/ui/SongsView.qml",
                                   {
                                       "containerItem": {id: model.objectId, payload: model.object},
                                       "songSearch": model.objectId,
                                       "album": "",
                                       "covers": [{art: model.art}],
                                       "isPlaylist": true,
                                       "genre": "",
                                       "pageTitle": qsTr("Playlist"),
                                       "line1": "",
                                       "line2": model.title
                                   })
            }
            else if (model.type === 5) {
                player.playFavorite(model, mainView.actionFinished); // play it
            }
        } else {
            delayFavoritePlayAll.model = model
            delayFavoritePlayAll.start()
        }
    }

    // Overlay to show when load failed
    Loader {
        anchors.fill: parent
        active: AllFavoritesModel.failure
        asynchronous: true
        sourceComponent: Component {
            DataFailureState {
                onReloadClicked: AllFavoritesModel.asyncLoad();
            }
        }
        visible: active
    }
}
