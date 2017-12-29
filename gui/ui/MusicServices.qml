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

import QtQuick 2.9
import QtQuick.Controls 2.2
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"


MusicPage {
    id: servicesPage
    objectName: "servicesPage"
    pageTitle: qsTr("My services")
    pageFlickable: serviceGrid //serviceGrid.visible ? serviceGrid : serviceList
/*
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
*/
/*
    MultiSelectListView {
        id: serviceList
        anchors {
            fill: parent
        }
        model: servicesModelFilter

        onStateChanged: {
            if (state === "multiselectable") {
                servicesPage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                servicesPage.state = "default"
            }
        }

        delegate: MusicListItem {
            id: service
            objectName: "servicesPageListItem" + index
            column: Column {
                Label {
                    id: serviceTitle
                    color: styleMusic.common.music
                    fontSize: "small"
                    objectName: "servicetitle"
                    text: model.title
                }

                Label {
                    id: serviceNickName
                    color: styleMusic.common.subtitle
                    fontSize: "x-small"
                    text: model.nickName
                }
            }

            height: units.gu(7)

            noCover: Qt.resolvedUrl("../graphics/radio.png")

            imageSource: model.id === "SA_RINCON65031_" ? Qt.resolvedUrl("../graphics/tunein.png") : model.icon

            multiselectable: false

            onItemClicked: {
                mainPageStack.push(Qt.resolvedUrl("Service.qml"),
                                   {
                                       "serviceItem": model,
                                       "pageTitle": model.title,
                                   })
            }
        }

        opacity: isListView ? 1.0 : 0.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }
*/
    MusicGridView {
        id: serviceGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)

        model: AllServicesModel
        //model: servicesModelFilter

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

            noCover: "qrc:/images/radio.png"
            coverSources: [{art: model.id === "SA_RINCON65031_" ? "qrc:/images/tunein.png" : model.icon}]

            onClicked: {
                stackView.push("qrc:/ui/Service.qml",
                                   {
                                       "serviceItem": model,
                                       "pageTitle": model.title
                                   })
            }
        }

        opacity: isListView ? 0.0 : 1.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

}

