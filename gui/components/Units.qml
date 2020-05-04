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

import QtQuick 2.9

Item {
    property real scaleFactor: 1.0
    property real fontScaleFactor: 1.0
    property real gridUnit: 8.0 * scaleFactor

    function dp(p) {
        return scaleFactor * p;
    }

    function gu(u) {
        return gridUnit * u;
    }

    function fs(s) {
        if (Android) {
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
        } else {
            if (s === "x-small")
                return 9.0 * scaleFactor * fontScaleFactor;
            if (s === "small")
                return 10.0 * scaleFactor * fontScaleFactor;
            if (s === "medium")
                return 12.0 * scaleFactor * fontScaleFactor;
            if (s === "large")
                return 14.0 * scaleFactor * fontScaleFactor;
            if (s === "x-large")
                return 16.0 * scaleFactor * fontScaleFactor;
        }
        return 0.0;
    }

}
