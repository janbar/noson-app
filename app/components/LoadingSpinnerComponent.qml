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

Item {
    id: refresh
    height: units.gu(5)
    width: parent.width
    visible: false

    property alias loadingText: loadingLabel.text

    anchors {
        horizontalCenter: parent.horizontalCenter
        top: parent.top
        margins: units.gu(20)
    }
    ActivityIndicator {
        id: loading
        objectName: "LoadingSpinner"
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: loadingLabel.left
        anchors.rightMargin: units.gu(1)
        running: refresh.visible
        z: 1
    }
    Label {
        id: loadingLabel
        text: i18n.tr("Loading...")
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: (loading.width / 2) + (loading.anchors.rightMargin / 2)
        anchors.verticalCenter: parent.verticalCenter
        fontSize: "large"
        color: styleMusic.common.subtitle
    }
}
