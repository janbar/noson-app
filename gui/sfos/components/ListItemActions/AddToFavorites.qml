/*
 * Copyright (C) 2016
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

MenuItem {
    property bool isFavorite: false
    property string description: ""
    property string art: ""
    property string iconSource: isFavorite ? "qrc:/images/starred.svg" : "qrc/images/non-starred.svg"

    text: isFavorite ? qsTr("Remove from favorites") : qsTr("Add to favorites")
    font.pointSize: units.fs("medium")
    height: visible ? implicitHeight : 0

    Component.onCompleted: {
        isFavorite = enabled && (AllFavoritesModel.findFavorite(model.payload) !== "")
    }

    onTriggered: {
        if (isFavorite && removeFromFavorites(model.payload))
            isFavorite = false
        else if (!isFavorite && addItemToFavorites(model, description, art))
            isFavorite = true
    }
}
