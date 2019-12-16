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

import "../../components"

Icon {
    height: units.gu(5)
    width: units.gu(20)
    source: "qrc:/images/media-playlist-shuffle.svg"
    label {
        //: this appears in a button with limited space (around 14 characters)
        text: qsTr("Shuffle")
        font.pixelSize: units.fx("medium")
        width: parent.width - units.gu(6)
        elide: Text.ElideRight
    }

    property var model

    onClicked: {
        delayShuffleModel.start()
    }

    Timer {
        id: delayShuffleModel
        interval: 100
        onTriggered: {
            var now = new Date();
            var seed = now.getSeconds();
            var index = Math.floor(model.count * Math.random(seed));
            if (index < model.count) {
                var item = model.get(index);
                var payload = makeItemPayload(item);
                if (player.isPlaying)
                    trackClicked({id: item.id, payload: payload}, false);
                else
                    trackClicked({id: item.id, payload: payload}, true); // play track
            }
        }
    }
}
