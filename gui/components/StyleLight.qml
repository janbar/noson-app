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

QtObject {
    property QtObject addtoPlaylist: QtObject {
        property color backgroundColor: "white"
        property color labelColor: "black"
        property color labelSecondaryColor: "#AAA"
        property color progressBackgroundColor: "white"
        property color progressForegroundColor: "#e95420"
        property color progressHandleColor: "black"
    }

    property QtObject common: QtObject {
        property color music: "black"
        property color subtitle: "dimgray"
        property color expandedColor: "white"
        property int albumSize: units.gu(10)
        property int itemHeight: units.gu(12)
        property int expandHeight: units.gu(7)
        property int expandedItem: units.gu(2)
        property int expandedTopMargin: units.gu(13.5)
        property int expandedLeftMargin: units.gu(2)
    }

    property QtObject dialog: QtObject {
        property color backgroundColor: "white"
        property color foregroundColor: "black"
        property color labelColor: "darkgray"
        property color confirmButtonColor: "green"
        property color confirmRemoveButtonColor: "red"
        property color cancelButtonColor: "darkgray"
    }

    property QtObject listView: QtObject {
        property color highlightColor: "lightgray"
    }

    property QtObject card: QtObject {
        property color backgroundColor: "#f2f2f2"
        property color borderColor: "#f2f2f2"
        property color labelColor: "black"
    }

    property QtObject mainView: QtObject{
        property color backgroundColor: "#f8f8f8"
        property color foregroundColor: "dimgray"
        property color footerColor: backgroundColor
        property color headerColor: backgroundColor
        property color labelColor: "black"
        property color normalTextFieldColor: "darkgrey"
        property color normalTextBaseColor: "black"
        property color selectedTextFieldColor: "darkgrey"
        property color selectedTextBaseColor: "black"
        property color highlightedColor: "#e95420"
        property color link: "green"
    }

    property QtObject nowPlaying: QtObject {
        property color backgroundColor: "transparent"
        property color foregroundColor: "#ebebeb"
        property color labelColor: "black"
        property color labelSecondaryColor: "dimgray"
        property color progressBackgroundColor: "#f8f8f8"
        property color progressForegroundColor: "#19b1e9"
    }

    property QtObject playerControls: QtObject {
        property color backgroundColor: "#f8f8f8"
        property color foregroundColor: "dimgray"
        property color labelColor: "black"
        property color progressBackgroundColor: "black"
        property color progressForegroundColor: "#e95420"
        property color progressHandleColor: "#e95420"
        property color volumeBackgroundColor: "black"
        property color volumeForegroundColor: "#e95420"
        property color volumeHandleColor: "black"
    }

    property QtObject popover: QtObject {
        property color backgroundColor: "black"
        property color foregroundColor: "dimgray"
        property color labelColor: "white"
    }

    property QtObject playlist: QtObject {
        property int infoHeight: units.gu(14);
        property int playlistItemHeight: units.gu(10);
        property int playlistAlbumSize: units.gu(8);
    }

    property QtObject toolbar: QtObject {
        property color fullBackgroundColor: "#f8f8f8"
        property color fullInnerPlayCircleColor: "#f8f8f8"
        property color fullOuterPlayCircleColor: "dimgray"
        property color fullProgressBackgroundColor: "dimgray"
        property color fullProgressTroughColor: "#e95420"
        property color foregroundColor: "dimgray"
        property color labelColor: "black"
    }

    property QtObject albums: QtObject {
        property int itemHeight: units.gu(4)
    }

    property QtObject artists: QtObject {
        property int itemHeight: units.gu(12.5)
    }

    Component.onCompleted: {
/*        theme.palette.normal.background = mainView.backgroundColor
        theme.palette.normal.base = common.subtitle
        theme.palette.normal.field = mainView.normalTextFieldColor
        theme.palette.normal.baseText = mainView.normalTextBaseColor
        theme.palette.selected.field = mainView.selectedTextFieldColor
        theme.palette.selected.baseText = mainView.selectedTextBaseColor*/
    }
}
