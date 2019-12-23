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

Item {
    property real scaleFactor: 1.0
    property real fontScaleFactor: 1.0
    property real gridUnit: scaleFactor * Screen.widthRatio * 12.0

    function dp(p) {
        return scaleFactor * p;
    }

    function gu(u) {
        return gridUnit * u;
    }

    function fs(s) {
        if (s === "x-small")
            return 10.0 * scaleFactor * fontScaleFactor;
        if (s === "small")
            return 12.0 * scaleFactor * fontScaleFactor;
        if (s === "medium")
            return 14.0 * scaleFactor * fontScaleFactor;
        if (s === "large")
            return 18.0 * scaleFactor * fontScaleFactor;
        if (s === "x-large")
            return 22.0 * scaleFactor * fontScaleFactor;

        return 0.0;
    }

    function fx(s) {
        if (s === "x-small")
            return Theme.fontSizeTiny * scaleFactor * fontScaleFactor;
        if (s === "small")
            return Theme.fontSizeExtraSmall * scaleFactor * fontScaleFactor;
        if (s === "medium")
            return Theme.fontSizeSmall * scaleFactor * fontScaleFactor;
        if (s === "large")
            return Theme.fontSizeMedium * scaleFactor * fontScaleFactor;
        if (s === "x-large")
            return Theme.fontSizeLarge * scaleFactor * fontScaleFactor;

        return 0.0;
    }

}
