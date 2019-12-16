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

import QtQuick 2.2
import Sailfish.Silica 1.0

import QtQuick.Layouts 1.1
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"


MusicPage {
    id: composersPage
    objectName: "composersPage"
    pageTitle: qsTr("Composers")
    pageFlickable: composerGridView
    searchable: true

    Component.onCompleted: {
        if (AllComposersModel.isNew()) {
            AllComposersModel.init(Sonos, "", false);
            AllComposersModel.asyncLoad();
        }
    }

    onSearchClicked: filter.visible = true

    header: MusicFilter {
        id: filter
        visible: false
    }

    MusicGridView {
        id: composerGridView
        itemWidth: units.gu(12)
        heightOffset: units.gu(7)
        clip: true
        model: SortFilterModel {
            model: AllComposersModel
            sort.property: "composer"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: Card {
            id: composerCard
            coverSources: makeCoverSource(undefined, model.composer, undefined)
            noCover: "qrc:/images/none.png"
            objectName: "composersPageGridItem" + index
            primaryText: model.composer !== undefined && model.composer !== "" ? model.composer : qsTr("Unknown Composer")
            secondaryTextVisible: false
            isFavorite: (AllFavoritesModel.findFavorite(model.payload).length > 0)

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onCountChanged: {
                    composerCard.isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            onClicked: {
                stackView.push("qrc:/ui/ComposerView.qml",
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "composerSearch": model.id,
                                       "composer": model.composer,
                                       "covers": [{art: composerCard.imageSource}],
                                       "pageTitle": qsTr("Composer")
                                   })
            }
            onPressAndHold: {
                if (isFavorite && removeFromFavorites(model.payload))
                    isFavorite = false
                else if (!isFavorite && addItemToFavorites(model, qsTr("Composer"), imageSource))
                    isFavorite = true
            }

            Component.onCompleted: {
                isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
            }
        }
    }
}
