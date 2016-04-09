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

import QtQuick 2.4
import Ubuntu.Components 1.3

Rectangle {
    id: nowPlayingSidebar
    anchors {
        fill: parent
    }
    color: styleMusic.nowPlaying.backgroundColor

    property alias flickable: queue  // fake normal Page
    property string title: ""  // fake normal Page

    Queue {
        id: queue
        anchors {
            fill: parent
            topMargin: 0
            bottomMargin: 0
        }
        clip: true
        header: Column {
            anchors {
                left: parent.left
                right: parent.right
            }
            Item {
                height: units.gu(10) // page header
                width: parent.width
            }
            NowPlayingFullView {
                anchors {
                    fill: undefined
                }
                clip: true
                height: units.gu(55)
                width: parent.width
            }
            NowPlayingToolbar {
                id: nowPlayingToolBar
                anchors {
                    fill: undefined
                }
                bottomProgressHint: false
                color: styleMusic.common.black
                height: units.gu(15)
                width: parent.width
            }
        }
    }
}
