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
import "../components/ListItemActions"


MusicPage {
    id: radiosPage
    objectName: "radiosPage"
    pageTitle: qsTr("My Radios")
    pageFlickable: radioGrid.visible ? radioGrid : radioList
    multiView: true
    searchable: true

    // used to detect view has updated properties since first load.
    // - isFavorite
    property bool taintedView: false

/*    SortFilterModel {
        id: radiosModelFilter
        model: AllRadiosModel
        sort.property: "title"
        sort.order: Qt.AscendingOrder
        sortCaseSensitivity: Qt.CaseInsensitive
        filter.property: "normalized"
        filter.pattern: new RegExp(normalizedInput(searchHeader.query), "i")
        filterCaseSensitivity: Qt.CaseInsensitive
    }*/

    MusicListView {
        id: radioList
        anchors.fill: parent
        model: AllRadiosModel
        delegate: MusicListItem {
            id: listItem

            property bool held: false
            onPressAndHold: held = true
            onReleased: held = false

            color: listItem.held ? "lightgrey" : "transparent"

            // check favorite on data loaded
            Connections {
                target: AllFavoritesModel
                onCountChanged: {
                    listItem.isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            noCover: "qrc:/images/radio.png"
            imageSource: model.icon !== "" ? model.icon
                       : model.streamId !== undefined && model.streamId !== "" ? "http://cdn-radiotime-logos.tunein.com/" + model.streamId + "q.png"
                       : ""
            description: qsTr("Radio")

            onImageError: model.icon = "" // reset invalid url from model
            onActionPressed: {
                radioClicked(model) // play radio
            }
            actionVisible: true
            actionIconSource: "qrc:/images/media-preview-start.svg"
            menuVisible: true

            menuItems: [
                AddToFavorites {
                    isFavorite: listItem.isFavorite
                    description: listItem.description
                    art: listItem.imageSource
                }
            ]

            column: Column {
                Label {
                    id: radioTitle
                    color: styleMusic.common.music
                    font.pointSize: units.fs("small")
                    text: model.title
                }

                Label {
                    id: radioGenre
                    color: styleMusic.common.subtitle
                    font.pointSize: units.fs("x-small")
                    text: model.streamId
                }
            }

            Component.onCompleted: {
                isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
            }
        }

        opacity: isListView ? 1.0 : 0.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

    MusicGridView {
        id: radioGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)

        model: AllRadiosModel

        delegate: Card {
            id: radioCard
            primaryText: model.title
            secondaryText: model.streamId

            isFavorite: (AllFavoritesModel.findFavorite(model.payload).length > 0)

            // check favorite on data updated
            Connections {
                target: AllFavoritesModel
                onLoaded: {
                    isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            noCover: "qrc:/images/radio.png"
            coverSources: [{art: model.icon}]

            onImageError: model.icon = "" // reset invalid url from model
            onClicked: {
                radioClicked(model) // play radio
            }
            onPressAndHold: {
                if (isFavorite && removeFromFavorites(model.payload))
                    isFavorite = false;
                else if (!isFavorite && addItemToFavorites(model, qsTr("Radio"), coverSources[0].art))
                    isFavorite = true;
                radiosPage.taintedView = true;
            }
        }

        opacity: isListView ? 0.0 : 1.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }
}

