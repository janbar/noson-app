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

QtObject {
    property QtObject addtoPlaylist: QtObject {
        property color backgroundColor: palette.base
        property color labelColor: palette.text
        property color labelSecondaryColor: palette.text
        property color progressBackgroundColor: palette.base
        property color progressForegroundColor: "#e95420"
        property color progressHandleColor: palette.text
    }

    property QtObject common: QtObject {
        property color music: palette.text
        property color subtitle: palette.text
        property int albumSize: units.gu(10)
        property int itemHeight: units.gu(12)
        property int expandHeight: units.gu(7)
        property int expandedItem: units.gu(2)
        property int expandedTopMargin: units.gu(13.5)
        property int expandedLeftMargin: units.gu(2)
    }

    property QtObject dialog: QtObject {
        property color backgroundColor: palette.base
        property color foregroundColor: palette.text
        property color labelColor: palette.text
        property color confirmButtonColor: "green"
        property color confirmRemoveButtonColor: "red"
        property color cancelButtonColor: palette.button
    }

    property QtObject card: QtObject {
        property color backgroundColor: palette.base
        property color borderColor: palette.text
        property color labelColor: palette.text
    }

    property QtObject mainView: QtObject{
        property color backgroundColor: palette.base
        property color foregroundColor: palette.text
        property color footerColor: backgroundColor
        property color headerColor: backgroundColor
        property color labelColor: palette.text
        property color normalTextFieldColor: palette.text
        property color normalTextBaseColor: palette.base
        property color selectedTextFieldColor: palette.highlight
        property color selectedTextBaseColor: palette.text
        property color highlightedColor: palette.highlight
        property color link: palette.link
    }

    property QtObject nowPlaying: QtObject {
        property color backgroundColor: "transparent"
        property color foregroundColor: palette.text
        property color labelColor: palette.text
        property color labelSecondaryColor: palette.text
        property color progressBackgroundColor: palette.base
        property color progressForegroundColor: palette.highlight
    }

    property QtObject playerControls: QtObject {
        property color backgroundColor: palette.base
        property color foregroundColor: palette.text
        property color labelColor: palette.text
        property color progressBackgroundColor: palette.text
        property color progressForegroundColor: palette.highlight
        property color progressHandleColor: progressForegroundColor
        property color volumeBackgroundColor: palette.text
        property color volumeForegroundColor: progressForegroundColor
        property color volumeHandleColor: palette.text
    }

    property QtObject popover: QtObject {
        property color backgroundColor: palette.shadow
        property color foregroundColor: palette.brightText
        property color labelColor: palette.brightText
    }

    property QtObject playlist: QtObject {
        property int infoHeight: units.gu(14);
        property int playlistItemHeight: units.gu(10);
        property int playlistAlbumSize: units.gu(8);
    }

    property QtObject albums: QtObject {
        property int itemHeight: units.gu(4)
    }

    property QtObject artists: QtObject {
        property int itemHeight: units.gu(12.5)
    }
}
