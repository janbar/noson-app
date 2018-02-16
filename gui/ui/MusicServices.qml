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
import "../components/ListItemActions"


MusicPage {
    id: servicesPage
    objectName: "servicesPage"
    pageTitle: qsTr("My Services")
    pageFlickable: serviceGrid.visible ? serviceGrid : serviceList
    multiView: true
    addVisible: true

    function removeService(type, serialNum) {
        var acls = deserializeACLS(settings.accounts);
        var _acls = []
        for (var i = 0; i < acls.length; ++i) {
            if (acls[i].type !== type || acls[i].sn !== serialNum)
                _acls.push(acls[i]);
        }
        customdebug("Remove service " + type + " with serial " + serialNum);
        Sonos.deleteServiceOAuth(type, serialNum);
        MyServicesModel.asyncLoad();
        settings.accounts = serializeACLS(_acls);
    }

    SortFilterModel {
        id: servicesModel
        model: MyServicesModel
        sort.property: "title"
        sort.order: Qt.AscendingOrder
        sortCaseSensitivity: Qt.CaseInsensitive
    }

    MusicListView {
        id: serviceList
        anchors.fill: parent
        model: servicesModel
        delegate: MusicListItem {
            id: serviceItem

            property bool held: false
            onPressAndHold: held = true
            onReleased: held = false

            color: serviceItem.held ? "lightgrey" : "transparent"

            noCover: "qrc:/images/radio.png"
            imageSource: model.id === "SA_RINCON65031_0" ? "qrc:/images/tunein.png" : model.icon
            description: qsTr("Service")

            onClicked: {
                stackView.push("qrc:/ui/Service.qml",
                                   {
                                       "serviceItem": model,
                                       "pageTitle": model.title
                                   })
            }

            actionVisible: false
            menuVisible: (model.id === "SA_RINCON65031_0" ? false : true)

            menuItems: [
                Remove {
                    onTriggered: {
                        removeService(model.type, model.serialNum)
                    }
                }
            ]

            column: Column {
                Label {
                    id: serviceTitle
                    color: styleMusic.common.music
                    font.pointSize: units.fs("small")
                    text: model.title
                }

                Label {
                    id: serviceNickName
                    color: styleMusic.common.subtitle
                    font.pointSize: units.fs("x-small")
                    text: model.nickName
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
        id: serviceGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)

        model: servicesModel

        delegate: Card {
            id: serviceCard
            primaryText: model.title
            secondaryText: model.nickName
            isFavorite: false

            noCover: "qrc:/images/radio.png"
            coverSources: [{art: model.id === "SA_RINCON65031_0" ? "qrc:/images/tunein.png" : model.icon}]

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

    onAddClicked: {
        stackView.push("qrc:/ui/AddService.qml")
    }

}

