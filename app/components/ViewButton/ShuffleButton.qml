/*
 * Copyright (C) 2013, 2014, 2015
 *      Andrew Hayzen <ahayzen@gmail.com>
 *      Daniel Holm <d.holmen@gmail.com>
 *      Victor Thompson <victor.thompson@gmail.com>
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
import Ubuntu.Components 1.2


Button {
    height: units.gu(4)
    strokeColor: UbuntuColors.green
    width: units.gu(15)

    property var model

    onClicked: shuffleModel(model)

    Text {
        anchors {
            centerIn: parent
        }
        color: "white"
        elide: Text.ElideRight
        height: parent.height
        horizontalAlignment: Text.AlignHCenter
        // TRANSLATORS: this appears in a button with limited space (around 14 characters)
        text: i18n.tr("Shuffle")
        verticalAlignment: Text.AlignVCenter
        width: parent.width - units.gu(2)
    }
}
