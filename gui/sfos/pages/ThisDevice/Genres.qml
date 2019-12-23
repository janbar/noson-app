/*
 * Copyright (C) 2019
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
import NosonMediaScanner 1.0
import "../../components"
import "../../components/Delegates"
import "../../components/Flickables"


MusicPage {
    id: genresPage
    objectName: "genresPage"
    pageFlickable: genreGridView
    searchable: true

    GenreList {
        id: genres
        Component.onCompleted: init()
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
            model: genres
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
            primaryText: (model.genre !== "<Undefined>" ? model.genre : tr_undefined)
            secondaryTextVisible: false

            onClicked: {
                pageStack.push("qrc:/sfos/pages/ThisDevice/SongsView.qml",
                                   {
                                       "covers": coverSources,
                                       "album": undefined,
                                       "genre": model.genre,
                                       "pageTitle": pageTitle,
                                       "line1": "",
                                       "line2": (model.genre !== "<Undefined>" ? model.genre : tr_undefined)
                                   })
            }
            onPressAndHold: {
            }
        }
    }
}
