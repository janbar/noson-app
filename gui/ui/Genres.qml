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
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"


MusicPage {
    id: genresPage
    objectName: "genresPage"
    pageTitle: qsTr("Genres")
    pageFlickable: genreGridView
    searchable: true

    MusicGridView {
        id: genreGridView
        itemWidth: units.gu(12)
        heightOffset: units.gu(7)
        model: AllGenresModel
/*        model: SortFilterModel {
            id: genresModelFilter
            model: AllGenresModel
            filter.property: "normalized"
            filter.pattern: searchHeader.query === "" ? /\S+/ : new RegExp(normalizedInput(searchHeader.query), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
            sort.property: "genre"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
        }*/

        property var artworks: []

        delegate: Card {
            id: genreCard
            coversGridVisible: true
            coverSources: []
            coverFlow: 4
            objectName: "genresPageGridItem" + index
            primaryText: model.genre || qsTr("<Undefined>")
            secondaryTextVisible: false

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onCountChanged: {
                    genreCard.isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            Component.onCompleted: {
                isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                // read from artworks cache
                var covers = genreGridView.artworks[model.genre];
                if (covers !== undefined)
                    coverSources = covers.slice(0);
                else {
                    coverBuilder.active = true;
                }
            }

            Connections {
                target: coverBuilder
                onStatusChanged: {
                    if (coverBuilder.status === Loader.Ready) {
                        genreGridView.artworks[model.genre] = coverBuilder.artwork.slice(0);
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
                                covers.push({art: makeCoverSource(childItem.art, childItem.artist, undefined), item: childItem});
                                ++index;
                            }
                            // unregister directory content
                            childModel.init(null, root);
                            return covers;
                        }
                    }
                }
            }

            // discard invalid art from covers
            onImageError: genreGridView.artworks[model.genre].splice(index, 1)

            onClicked: {
                stackView.push("qrc:/ui/SongsView.qml",
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "songSearch": model.id + "//",
                                       "covers": coverSources,
                                       "album": "",
                                       "genre": model.genre,
                                       "pageTitle": qsTr("Genre"),
                                       "line1": "",
                                       "line2": model.genre
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
}
