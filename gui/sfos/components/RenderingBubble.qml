/*
 * Copyright (C) 2016-2019
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

Loader {
    id: loader
    asynchronous: true
    active: false
    width: parent.width

    property real maxWidth: parent.width
    property real maxHeight: mainView.height - units.gu(32)
    property real contentHeight:  units.gu(4) + player.renderingModel.count * (item ? item.rowHeight : 0)
    property real preferedHeight: contentHeight > maxHeight ? maxHeight : contentHeight
    property bool opened: false

    onActiveChanged: {
        if (!active)
            opened = false
    }

    height: opened ? preferedHeight : 0
    visible: height > 0

    Behavior on height {
        SmoothedAnimation { velocity: 20 * preferedHeight }
    }

    function open() {
        if (status !== Loader.Ready)
            loader.loaded.connect(open);
        else {
            player.refreshRendering()
            opened = true;
        }
    }

    function close() {
        opened = false;
    }

    sourceComponent: Component {
        RenderingControlerView {
            id: renderingControlerView
            backgroundColor: "transparent"
            foregroundColor: styleMusic.view.foregroundColor
            labelColor: styleMusic.view.labelColor
            anchors.fill: loader
            anchors.topMargin: units.gu(2)
            anchors.bottomMargin: units.gu(2)
        }
    }
}
