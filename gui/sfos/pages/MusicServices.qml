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

    /*pageMenuItems: [
            MenuItem { text: qsTr("About"); onClicked: dialogAbout.open() },   
            Repeater {
                model: tabs

                MenuItem {
                    text: model.title
                    onClicked: {
                        pageStack.push(model.source)
                    }
                }
            }
    ]*/
        
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
            imageSources: [{art: model.type === "65031" ? "qrc:/images/tunein.png" : model.icon}]
            description: qsTr("Service")

            onClicked: {
                pageStack.push("qrc:/sfos/pages/Service.qml",
                                   {
                                       "serviceItem": model,
                                       "pageTitle": model.title
                                   })
            }

            actionVisible: false
            menuVisible: (model.id === "SA_RINCON65031_0" ? false : true)

            menu: ContextMenu {
                Remove {
                    onClicked: {
                        removeService(model.type, model.serialNum)
                    }
                }
            }

            column: Column {
                Label {
                    id: serviceTitle
                    color: styleMusic.view.primaryColor
                    font.pointSize: units.fs("medium")
                    text: model.title
                }

                Label {
                    id: serviceNickName
                    color: styleMusic.view.secondaryColor
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
        heightOffset: units.gu(9)

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

        opacity: isListView ? 0.0 : 1.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
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
            var newMenuItem = menuItemComp.createObject(pageMenu, {"text" : tabs.get(i).title, "source" : tabs.get(i).source})
            newMenuItem.onClicked.connect(tabs.pushPage)
        }
    }

    
    
    
}

