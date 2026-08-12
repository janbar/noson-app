/*
 * Copyright (C) 2026
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

// YouTube Music via the Sonos cloud content API, laid out like the other
// service browse pages (Service.qml): the app header drives up-navigation
// (‹ at the root / ^ when drilled in via goUpClicked), a list/grid toggle,
// and a search dialog. Sign-in runs an embedded Sonos login (QtWebEngine).

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtWebEngine
import NosonApp 1.0
import "components"
import "components/Delegates"
import "components/Flickables"

MusicPage {
    id: ytPage
    objectName: "youtubeMusicPage"
    pageTitle: qsTr("YouTube Music")

    readonly property bool signedIn: SonosAccount.status === SonosAccount.Ready
    property bool loggingIn: false

    // Root when signed-out (header shows ‹ to leave) or at the service home;
    // otherwise the header shows ^ and calls goUpClicked.
    isRoot: !signedIn || cloudModel.isRoot
    multiView: signedIn && !loggingIn
    searchable: signedIn && !loggingIn
    pageFlickable: mediaGrid.visible ? mediaGrid : mediaList

    CloudMediaModel {
        id: cloudModel
        onError: function(message) { console.log("YouTube Music: " + message) }
    }

    Connections {
        target: SonosAccount
        function onLoginSucceeded() { ytPage.loggingIn = false }
        function onNeedLogin() { ytPage.loggingIn = false }
        function onStatusChanged() {
            if (SonosAccount.status === SonosAccount.Ready && cloudModel.count === 0) {
                // (Re)bind the discovered household now that the system is up,
                // then discover the account and load the home page.
                cloudModel.init(SonosAccount, Sonos.museHouseholdId())
                cloudModel.start()
            }
        }
    }

    Component.onCompleted: {
        cloudModel.init(SonosAccount, Sonos.museHouseholdId())
        SonosAccount.restore()
        if (settings.preferListView)
            isListView = true
    }

    onGoUpClicked: cloudModel.back()
    onListViewClicked: settings.preferListView = isListView
    onSearchClicked: searchDialog.open()

    function clickItem(model) {
        if (model.isContainer)
            cloudModel.browse(model.objectId, model.title, model.type, model.href)
        else
            trackClicked(model, true) // play the track
    }

    function playItem(model) {
        if (!model.canPlay)
            return
        if (model.isContainer)
            playAll(model)      // enqueue whole album/playlist
        else
            trackClicked(model, true)
    }

    // ---- Signed-out / expired prompt ----
    Column {
        anchors.centerIn: parent
        width: parent.width * 0.8
        spacing: units.gu(2)
        visible: !ytPage.signedIn && !ytPage.loggingIn

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            color: styleMusic.view.primaryColor
            text: SonosAccount.status === SonosAccount.Expired
                  ? qsTr("Your YouTube Music session expired. Please sign in again.")
                  : qsTr("Sign in to your Sonos account to use YouTube Music.")
        }
        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Sign in")
            onClicked: { SonosAccount.beginLogin(); ytPage.loggingIn = true }
        }
    }

    // ---- Embedded Sonos login ----
    WebEngineView {
        id: loginView
        anchors.fill: parent
        visible: ytPage.loggingIn
        url: ytPage.loggingIn ? SonosAccount.signinUrl : "about:blank"
        Component.onCompleted: SonosAccount.beginLogin()
        onLoadingChanged: function(loadRequest) {
            if (loadRequest.status === WebEngineView.LoadSucceededStatus) {
                var u = loadRequest.url.toString();
                if (u.indexOf("play.sonos.com") !== -1 && u.indexOf("/login") === -1)
                    SonosAccount.harvestCookies();
            }
        }
    }

    // ---- List view ----
    MusicListView {
        id: mediaList
        anchors.fill: parent
        visible: ytPage.signedIn && !ytPage.loggingIn && isListView
        model: cloudModel
        delegate: MusicListItem {
            id: listItem
            noCover: "qrc:/images/no_cover.png"
            imageSources: model.art !== "" ? [{art: model.art}] : [{art: "qrc:/images/no_cover.png"}]
            description: model.subtitle
            onClicked: clickItem(model)
            // Play triangle for playable rows (album/playlist/track)
            actionVisible: model.canPlay
            actionIconSource: "qrc:/images/media-preview-start.svg"
            onActionPressed: playItem(model)
            height: units.gu(8)
            column: Column {
                Label {
                    color: styleMusic.view.primaryColor
                    font.pointSize: units.fs("medium")
                    text: model.title
                }
                Label {
                    color: styleMusic.view.secondaryColor
                    font.pointSize: units.fs("x-small")
                    text: model.subtitle
                    visible: text !== ""
                }
            }
        }
    }

    // ---- Grid view ----
    MusicGridView {
        id: mediaGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9)
        visible: ytPage.signedIn && !ytPage.loggingIn && !isListView
        model: cloudModel
        delegate: Card {
            height: mediaGrid.cellHeight
            width: mediaGrid.cellWidth
            primaryText: model.title
            secondaryText: model.subtitle
            isFavorite: false
            canPlay: model.canPlay
            overlay: false
            noCover: "qrc:/images/no_cover.png"
            coverSources: model.art !== "" ? [{art: model.art}] : [{art: "qrc:/images/no_cover.png"}]
            onClicked: clickItem(model)
            onPlayClicked: playItem(model)
        }
    }

    // ---- Loading spinner ----
    BusyIndicator {
        anchors.centerIn: parent
        width: units.gu(8)
        height: units.gu(8)
        running: cloudModel.busy
        visible: cloudModel.busy && ytPage.signedIn && !ytPage.loggingIn
        z: 1
    }

    // ---- Search dialog (opened from the header search button) ----
    Dialog {
        id: searchDialog
        modal: true
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.9, units.gu(50))
        title: qsTr("Search YouTube Music")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onOpened: { searchField.text = ""; searchField.forceActiveFocus() }
        onAccepted: if (searchField.text.length > 0) cloudModel.search(searchField.text)
        contentItem: TextField {
            id: searchField
            placeholderText: qsTr("Artist, album, song or playlist")
            onAccepted: searchDialog.accept()
        }
    }
}
