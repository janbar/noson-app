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
import QtQuick.Controls 2.2
import NosonApp 1.0


DialogBase {
    id: dialog
    title: qsTr("About")

    footer: Row {
        leftPadding: units.gu(1)
        rightPadding: units.gu(1)
        bottomPadding: units.gu(1)
        spacing: units.gu(1)
        layoutDirection: Qt.RightToLeft

        Button {
            flat: true
            text: qsTr("Close")
            onClicked: dialog.reject()
        }
    }

    contentSpacing: units.gu(1)

    Text {
        color: styleMusic.dialog.foregroundColor
        width: dialog.availableWidth
        text: qsTr("The project has started in 2015 and is intented to make a fast and smart controller for your SONOS devices."
                   + " You can browse your music library and play track or radio on any zones."
                   + " You can manage grouping zones, queue, and playlists, and fully control the playback.")
        wrapMode: Label.Wrap
        font.pointSize: units.fs("medium")
    }
    Text {
        color: styleMusic.dialog.foregroundColor
        width: dialog.availableWidth
        text: qsTr("Author: %1").arg("Jean-Luc Barriere")
        font.pointSize: units.fs("medium")
    }
    Text {
        color: styleMusic.dialog.foregroundColor
        width: dialog.availableWidth
        text: qsTr("Version: %1").arg(VersionString) + " (libnoson " + Sonos.getLibVersion() + ")"
        font.pointSize: units.fs("medium")
    }
    Text {
        id: donate
        color: styleMusic.dialog.foregroundColor
        width: dialog.availableWidth
        font.pointSize: units.fs("medium")
        text: "<a href='https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=jlbarriere68%40gmail%2ecom&lc=US&item_name=noson%2dapp&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted'>Donate with Paypal</a>"
        onLinkHovered: {
            if (hoveredLink)
                font.bold = true;
            else
                font.bold = false;
        }
        onLinkActivated: Qt.openUrlExternally(link)
        linkColor: styleMusic.view.linkColor
    }
}
