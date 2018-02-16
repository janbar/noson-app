/*
 * Copyright (C) 2018
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
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"


MusicPage {
    id: addServicePage
    objectName: "addServicePage"

    property bool isListView: false

    pageTitle: i18n.tr("Add service")
    pageFlickable: serviceGrid
    searchable: true
    searchResultsCount: servicesModelFilter.count
    state: "default"
    states: [
        SearchableHeadState {
            thisPage: addServicePage
            searchEnabled: AllServicesModel.count > 0
            thisHeader {
                extension: DefaultSections { }
            }
        },
        SearchHeadState {
            id: searchHeader
            thisPage: addServicePage
            thisHeader {
                extension: DefaultSections { }
            }
        }
    ]

    width: mainPageStack.width

    SortFilterModel {
        id: servicesModelFilter
        model: AllServicesModel
        sort.property: "title"
        sort.order: Qt.AscendingOrder
        sortCaseSensitivity: Qt.CaseInsensitive
        filter.property: "normalized"
        filter.pattern: new RegExp(normalizedInput(searchHeader.query), "i")
        filterCaseSensitivity: Qt.CaseInsensitive
    }

    MusicGridView {
        id: serviceGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)

        model: servicesModelFilter

        onStateChanged: {
            if (state === "multiselectable") {
                servicesPage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                servicesPage.state = "default"
            }
        }

        delegate: Card {
            id: serviceCard
            primaryText: model.title
            secondaryText: model.nickName
            isFavorite: false

            noCover: Qt.resolvedUrl("../graphics/radio.png")
            coverSources: [{art: model.type === "65031" ? "qrc:/graphics/tunein.png" : model.icon}]

            onClicked: {
                if (model.type !== "65031") {
                     var serialNum = 0;
                     var acls = deserializeACLS(startupSettings.accounts);
                     for (var i = 0; i < acls.length; ++i) {
                         if (acls[i].type === model.type && parseInt(acls[i].sn) >= serialNum)
                             serialNum = parseInt(acls[i].sn) + 1;
                     }
                     customdebug("Register service " + model.title + " (" + model.type + ") with serial " + serialNum);
                     Sonos.addServiceOAuth(model.type, serialNum, "", "", "");
                     MyServicesModel.asyncLoad();
                     acls.push({type: model.type, sn: serialNum, key: "", token: "", username: ""});
                     startupSettings.accounts = serializeACLS(acls);
                }
                mainPageStack.goBack();
            }
        }
    }

}

