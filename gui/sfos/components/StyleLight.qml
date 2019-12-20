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

QtObject {
    property QtObject view: QtObject {
        property color foregroundColor: Theme.primaryColor
        property color backgroundColor: Theme.overlayBackgroundColor
        property color highlightedColor: Theme.highlightColor
        property color labelColor: Theme.primaryColor
        property color primaryColor: Theme.primaryColor
        property color secondaryColor: Theme.primaryColor
        property color linkColor: Theme.secondaryColor
        property color footerColor: backgroundColor
        property color headerColor: backgroundColor

        property int albumSize: units.gu(10)
        property int expandHeight: units.gu(7)
    }

    property QtObject dialog: QtObject {
        property color backgroundColor: Theme.overlayBackgroundColor
        property color foregroundColor: Theme.primaryColor
        property color labelColor: Theme.primaryColor
        property color confirmButtonColor: "green"
        property color confirmRemoveButtonColor: "red"
        property color cancelButtonColor: Theme.secondaryColor
    }

    property QtObject card: QtObject {
        property color backgroundColor: "grey"
        property color foregroundColor: Theme.primaryColor
        property color borderColor: backgroundColor
    }

    property QtObject nowPlaying: QtObject {
        property color backgroundColor: "transparent"
        property color foregroundColor: Theme.primaryColor
        property color primaryColor: Theme.primaryColor
        property color secondaryColor: Theme.primaryColor
    }

    property QtObject playerControls: QtObject {
        property color backgroundColor: Theme.overlayBackgroundColor
        property color foregroundColor: Theme.primaryColor
        property color labelColor: Theme.primaryColor
        property color progressBackgroundColor: "grey"
        property color progressForegroundColor: Theme.highlightColor
        property color progressHandleColor: Theme.primaryColor
        property color volumeBackgroundColor: "grey"
        property color volumeForegroundColor: Theme.primaryColor
        property color volumeHandleColor: Theme.primaryColor
    }

    property QtObject popover: QtObject {
        property color backgroundColor: Theme.overlayBackgroundColor
        property color foregroundColor: Theme.primaryColor
        property color labelColor: Theme.primaryColor
    }
}
