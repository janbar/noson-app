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
import "components"
import "components/Delegates"
import "components/Flickables"
import "components/ListItemActions"


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
            imageSources: model.type === FavoritesModel.TypeAlbum ? makeCoverSource(model.art, model.artist, model.title)
                        : model.type === FavoritesModel.TypePerson ? makeCoverSource(undefined, model.artist, undefined)
                        : model.type === FavoritesModel.TypeGenre ? [{art: "qrc:/images/no_cover.png"}]
                        : model.type === FavoritesModel.TypeAudioItem && model.canQueue ? makeCoverSource(model.art, model.author, model.album)
                        : model.type === FavoritesModel.TypeAudioItem && model.art === "" ? [{art: "qrc:/images/radio.png"}]
                        : makeCoverSource(model.art, undefined, undefined)
            description:  model.description.length > 0 ? model.description
                       : model.type === FavoritesModel.TypeAlbum ? qsTr("Album")
                       : model.type === FavoritesModel.TypePerson ? qsTr("Artist")
                       : model.type === FavoritesModel.TypeGenre ? qsTr("Genre")
                       : model.type === FavoritesModel.TypePlaylist ? qsTr("Playlist")
                       : model.type === FavoritesModel.TypeAudioItem ? qsTr("Song")
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
            function onLoaded(succeeded) {
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
            height: favoriteGrid.cellHeight
            width: favoriteGrid.cellWidth
            primaryText: model.title
            secondaryText: model.description.length > 0 ? model.description
                         : model.type === FavoritesModel.TypeAlbum ? qsTr("Album")
                         : model.type === FavoritesModel.TypePerson ? qsTr("Artist")
                         : model.type === FavoritesModel.TypeGenre ? qsTr("Genre")
                         : model.type === FavoritesModel.TypePlaylist ? qsTr("Playlist")
                         : model.type === FavoritesModel.TypeAudioItem ? qsTr("Song")
                         : ""
            overlay: false // item icon could be transparent
            noCover: model.type === FavoritesModel.TypeAudioItem && !model.canQueue ? "qrc:/images/radio.png"
                   : model.type === FavoritesModel.TypePerson ? "qrc:/images/none.png"
                   : "qrc:/images/no_cover.png"
            coverSources: model.type === FavoritesModel.TypeAlbum ? makeCoverSource(model.art, model.artist, model.title)
                        : model.type === FavoritesModel.TypePerson ? makeCoverSource(undefined, model.artist, undefined)
                        : model.type === FavoritesModel.TypeGenre ? [{art: "qrc:/images/no_cover.png"}]
                        : model.type === FavoritesModel.TypeAudioItem && model.canQueue ? makeCoverSource(model.art, model.author, model.album)
                        : model.type === FavoritesModel.TypeAudioItem && model.art === "" ? [{art: "qrc:/images/radio.png"}]
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

    function clickItem(model) {
        if (!model.isService) {
            if (model.type === 1) {
                stackView.push("qrc:/controls2/Library.qml",
                               {
                                   "rootPath": model.objectId,
                                   "rootTitle": model.album,
                                   "rootType": LibraryModel.NodeAlbum,
                                   "isListView": true,
                                   "displayType": LibraryModel.DisplayTrackList,
                                   "nodeItem": makeContainerItem(model)
                               })
            }
            else if (model.type === 2) {
                stackView.push("qrc:/controls2/Library.qml",
                               {
                                   "rootPath": model.objectId,
                                   "rootTitle": model.artist,
                                   "rootType": LibraryModel.NodePerson,
                                   "isListView": false,
                                   "displayType": LibraryModel.DisplayGrid,
                                   "nodeItem": makeContainerItem(model)
                               })
            }
            else if (model.type === 3) {
                stackView.push("qrc:/controls2/Library.qml",
                               {
                                   "rootPath": model.objectId,
                                   "rootTitle": model.title,
                                   "rootType": LibraryModel.NodeGenre,
                                   "isListView": false,
                                   "displayType": LibraryModel.DisplayEditorial,
                                   "nodeItem": makeContainerItem(model)
                               })
            }
            else if (model.type === 4) {
                stackView.push("qrc:/controls2/SongsView.qml",
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

    Component.onCompleted: {
        if (settings.preferListView)
            isListView = true
    }

    onListViewClicked: {
        settings.preferListView = isListView
    }
}
