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
    pageMenuEnabled: true
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
        clip: true
        delegate: MusicListItem {
            id: serviceItem

            color: "transparent"

            noCover: "qrc:/images/radio.png"
            imageSources: [{art: model.type === "65031" ? "qrc:/images/tunein.png" : model.icon}]
            description: qsTr("Service")

            onClicked: {
                pageStack.push("qrc:/silica/pages/Service.qml",
                                   {
                                       "serviceItem": model,
                                       "pageTitle": model.title
                                   })
            }

            actionVisible: false

            menu: ContextMenu {
                hasContent: (model.id === "SA_RINCON65031_" ? false : true)
                Remove {
                    onClicked: {
                        removeService(model.type, model.serialNum)
                    }
                }
            }

            coverSize: units.gu(5)

            column: Column {
                Label {
                    id: serviceTitle
                    color: styleMusic.view.primaryColor
                    font.pixelSize: units.fx("medium")
                    text: model.title
                }

                Label {
                    id: serviceNickName
                    color: styleMusic.view.secondaryColor
                    font.pixelSize: units.fx("x-small")
                    text: model.nickName
                    visible: text !== ""
                }
            }
        }

        visible: isListView
    }

    MusicGridView {
        id: serviceGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9)
        clip: true
        model: servicesModel

        delegate: Card {
            id: serviceCard
            primaryText: model.title
            secondaryText: model.nickName
            isFavorite: false

            overlay: false // item icon could be transparent
            noCover: "qrc:/images/radio.png"
            coverSources: [{art: model.type === "65031" ? "qrc:/images/tunein.png" : model.icon}]

            onClicked: {
                console.log("Clicked card")
                pageStack.push("Service.qml",
                                   {
                                       "serviceItem": model,
                                       "pageTitle": model.title
                                   })
            }
        }

        visible: !isListView
    }
    
    onAddClicked: {
        pageStack.push("AddService.qml")
    }
    
    Component {
        id: menuItemComp
        MenuItem {
            property string source: ""
            onClicked: {
                tabs.pushPage(source)
            }
        }
    }

    Component.onCompleted: {
        console.debug("Populating menu")
        for (var i=0; i< tabs.rowCount() ; i++){
            var newMenuItem = menuItemComp.createObject(pageMenuContent, {"text" : tabs.get(i).title, "source" : tabs.get(i).source})
            newMenuItem.onClicked.connect(tabs.pushPage)
        }
    }

    
    
    
}

