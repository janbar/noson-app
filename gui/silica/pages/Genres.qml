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
import QtQuick.Layouts 1.1
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"


MusicPage {
    id: genresPage
    objectName: "genresPage"
    pageTitle: qsTr("Genres")
    pageFlickable: genreGridView
    pageMenuEnabled: false
    searchable: true

    Component.onCompleted: {
        if (AllGenresModel.isNew()) {
            AllGenresModel.init(Sonos, "", false);
            AllGenresModel.asyncLoad();
        }
    }

    onSearchClicked: filter.visible = true

    //Header
    MusicFilter {
        id: filter
        visible: false
        onVisibleChanged: showToolbar = !visible
    }

    MusicGridView {
        id: genreGridView
        anchors.topMargin: filter.visible ? filter.height : 0
        itemWidth: units.gu(12)
        heightOffset: units.gu(7)
        clip: true
        model: SortFilterModel {
            model: AllGenresModel
            sort.property: "genre"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        property int delayed: 0

        delegate: Card {
            id: genreCard
            coversGridVisible: true
            coverSources: []
            coverFlow: 4
            objectName: "genresPageGridItem" + index
            primaryText: model.genre || tr_undefined
            secondaryTextVisible: false

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onLoaded: {
                    genreCard.isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            Component.onCompleted: {
                isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                // read from artworks cache
                var covers = mainView.genreArtworks[model.genre];
                if (covers !== undefined)
                    coverSources = covers.slice(0);
                else {
                    delayCoverBuilder.start();
                }
            }

            Timer {
                id: delayCoverBuilder
                interval: 200
                // Postpone triggering regarding current delayed count
                onRunningChanged: {
                    if (running) {
                        interval += 300 * genreGridView.delayed++
                    }
                }
                onTriggered: {
                    genreGridView.delayed--;
                    coverBuilder.active = true;
                }
                Component.onDestruction: {
                    if (running) {
                        stop();
                        genreGridView.delayed--;
                    }
                }
            }

            Connections {
                target: coverBuilder
                onStatusChanged: {
                    if (coverBuilder.status === Loader.Ready) {
                        mainView.genreArtworks[model.genre] = coverBuilder.artwork.slice(0);
                        genreCard.coverSources = coverBuilder.artwork.slice(0);
                    }
                }
            }

            Loader {
                id: coverBuilder
                active: false
                asynchronous: true
                property var artwork: []
                sourceComponent: Component {
                    Item {
                        Component.onCompleted: artwork = findCoverSources(model)

                        AlbumsModel {
                            id: childModel
                        }

                        function findCoverSources(modelItem) {
                            var covers = [];
                            var root = modelItem.id + "/";
                            // register and load directory content for root
                            childModel.init(Sonos, root, true);
                            childModel.resetModel();
                            var count = childModel.count;
                            var index = 0;
                            while (index < count && index < 4) {
                                var childItem = childModel.get(index);
                                covers.push({art: makeArt(childItem.art, childItem.artist, undefined), item: childItem});
                                ++index;
                            }
                            // unregister directory content
                            childModel.init(null, root);
                            return covers;
                        }
                    }
                }
            }

            // discard invalid url in artwork cache
            onImageError: mainView.genreArtworks[model.genre].splice(index, 1)

            onClicked: {
                pageStack.push("qrc:/silica/pages/SongsView.qml",
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "songSearch": model.id + "//",
                                       "covers": coverSources,
                                       "album": undefined,
                                       "genre": model.genre,
                                       "pageTitle": qsTr("Genre"),
                                       "line1": "",
                                       "line2": model.genre || tr_undefined
                                   })
            }
            onPressAndHold: {
                if (isFavorite && removeFromFavorites(model.payload))
                    isFavorite = false
                else if (!isFavorite && addItemToFavorites(model, qsTr("Genre"), imageSource))
                    isFavorite = true
            }
        }
    }

    // Overlay to show when load failed
    Loader {
        anchors.fill: parent
        active: AllGenresModel.failure
        asynchronous: true
        sourceComponent: Component {
            DataFailureState {
                onReloadClicked: AllGenresModel.asyncLoad();
            }
        }
        visible: active
    }
}
