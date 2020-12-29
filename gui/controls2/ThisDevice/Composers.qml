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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import NosonApp 1.0
import NosonMediaScanner 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"


MusicPage {
    id: composersPage
    objectName: "composersPage"
    pageFlickable: composerGridView
    searchable: true

    ComposerList {
        id: composers
        Component.onCompleted: init()
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

        model: SortFilterModel {
            model: composers
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
            primaryText: (model.composer !== "<Undefined>" ? model.composer : tr_undefined)
            secondaryTextVisible: false

            onClicked: {
                stackView.push("qrc:/controls2/ThisDevice/ComposerView.qml",
                                   {
                                       "composer": model.composer,
                                       "covers": [{art: composerCard.imageSource}],
                                       "pageTitle": pageTitle
                                   })
            }
        }
    }
}
