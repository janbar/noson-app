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
import "../components/Dialog"


MusicPage {
    id: addServicePage
    objectName: "addServicePage"
    pageTitle: qsTr("Add Service")
    pageFlickable: serviceGrid
    pageMenuEnabled: false
    searchable: false

    MusicGridView {
        id: serviceGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(7)
        clip: true

        model: SortFilterModel {
            model: AllServicesModel
            sort.property: "title"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            //TODO? filter.pattern: new RegExp(normalizedInput(mainView.query), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: Card {
            id: serviceCard
            primaryText: model.title
            secondaryTextVisible: false
            isFavorite: false

            noCover: ""
            coverSources: [{art: model.type === "65031" ? "qrc:/images/tunein.png" : model.icon}]

            onClicked: {
                if (model.type !== "65031") {
                    // for UserId the label is username
                    if (model.auth !== "UserId") {
                        dialogServiceLabel.open(model.type, model.icon);
                    } else {
                        createAccount(model.type, "");
                        pageStack.pop();
                    }
               } else {
                   pageStack.pop();
               }
            }
        }
    }

    function createAccount(serviceType, label) {
        var serialNum = 0;
        var acls = deserializeACLS(settings.accounts);
        for (var i = 0; i < acls.length; ++i) {
            if (acls[i].type === serviceType && parseInt(acls[i].sn) >= serialNum) {
                serialNum = parseInt(acls[i].sn) + 1;
            }
        }
        customdebug("Register service " + serviceType + " '" + label + "' with serial " + serialNum);
        Sonos.addServiceOAuth(serviceType, serialNum, "", "", label);
        MyServicesModel.asyncLoad();
        acls.push({type: serviceType, sn: serialNum, key: "", token: "", username: label});
        settings.accounts = serializeACLS(acls);
    }

    Connections {
        target: dialogServiceLabel
        onDone: {
            if (dialogServiceLabel.result === DialogResult.Accepted) {
                createAccount(dialogServiceLabel.serviceType, dialogServiceLabel.label);
                afterServiceLabel.start();
            }
        }
    }

    Timer {
        id: afterServiceLabel
        interval: 50
        onTriggered: {
            if (dialogServiceLabel.result === DialogResult.Accepted) {
                if (dialogServiceLabel.status !== DialogStatus.Closed)
                    restart();
                else
                    pageStack.pop();
            }
        }
    }
}

