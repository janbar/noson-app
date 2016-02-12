/*
 * Copyright (C) 2015 Canonical, Ltd.
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

Page {
    id: page

    property Page   bottomEdgePage: null
    property alias  bottomEdgeTitle: tipLabel.text
    property color  bottomEdgeBackgroundColor: styleMusic.mainView.backgroundColor
    property color  bottomEdgeTipColor: styleMusic.mainView.backgroundColor
    property color  bottomEdgeTitleColor: styleMusic.mainView.labelColor
    property bool   bottomEdgeEnabled: true
    property bool   bottomEdgeHeaderHidden: true
    property bool   bottomEdgeUseHeaderInCalculations: true
    property real   bottomEdgeExpandThreshold: 0.2
    property real   bottomEdgeMarging: 0

    readonly property bool isReady: bottomEdgePageLoaded && bottomEdgePage.active && bottomEdge.y === __pageStartY
    readonly property bool isCollapsed: (bottomEdge.y === page.height)
    readonly property bool bottomEdgePageLoaded: bottomEdgePage != null

    readonly property int __pageStartY: bottomEdgeUseHeaderInCalculations ? fakeHeader.height : 0

    signal bottomEdgeReleased()
    signal bottomEdgeDismissed()

    function __pushPage() {
        if (bottomEdgePageLoaded) {
            bottomEdgePage.active = true
            page.pageStack.push(bottomEdgePage)
            if (bottomEdgePage.flickable) {
                bottomEdgePage.flickable.contentY = -page.header.height
                bottomEdgePage.flickable.returnToBounds()
            }
            if (bottomEdgePage.ready)
                bottomEdgePage.ready()
        }
    }

    onActiveChanged: {
        if (active) {
            bottomEdge.state = "collapsed"
        }
    }

    onBottomEdgePageChanged: {
        tip.forceActiveFocus()
        if (page.isReady && bottomEdgePageLoaded && !bottomEdgePage.active) {
            page.__pushPage()
        }
    }

    Rectangle {
        id: bgVisual

        color: styleMusic.common.black
        anchors {
            fill: parent
            topMargin: __pageStartY
        }
        opacity: 0.7 * ((page.height - bottomEdge.y) / page.height)
        z: 1
    }

    Rectangle {
        id: shadow

        anchors {
            left: parent.left
            right: parent.right
            bottom: bottomEdge.top
        }
        height: units.gu(1)
        z: 1
        opacity: 0.0
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.2) }
        }
    }

    MouseArea {
        id: mouseArea

        property real previousY: -1
        property string dragDirection: "None"

        preventStealing: true
        drag {
            axis: Drag.YAxis
            target: bottomEdge
            minimumY: __pageStartY
            maximumY: page.height
        }
        enabled: bottomEdgePageLoaded
        visible: page.bottomEdgeEnabled

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            bottomMargin: bottomEdgeMarging
        }
        height: bottomEdge.tipHeight
        z: 1

        onReleased: {
            page.bottomEdgeReleased()
            if ((dragDirection === "BottomToTop") &&
                    bottomEdge.y < (page.height - page.height * bottomEdgeExpandThreshold - bottomEdge.tipHeight)) {
                bottomEdge.state = "expanded"
            } else {
                bottomEdge.state = "collapsed"
            }
            previousY = -1
            dragDirection = "None"
        }

        onMouseYChanged: {
            var yOffset = previousY - mouseY
            // skip if was a small move
            if (Math.abs(yOffset) <= units.gu(2))
                return

            previousY = mouseY
            dragDirection = yOffset > 0 ? "BottomToTop" : "TopToBottom"
        }
    }

    FakeHeader {
        id: fakeHeader
        visible: bottomEdgeEnabled && !bottomEdgeHeaderHidden

        anchors {
            left: parent.left
            right: parent.right
        }
        y: -fakeHeader.height + (fakeHeader.height * (page.height - bottomEdge.y)) / (page.height - fakeHeader.height)
        z: bgVisual.z + 1

        Behavior on y {
            UbuntuNumberAnimation {
                duration: UbuntuAnimation.SnapDuration
            }
        }
    }

    Rectangle {
        id: tip

        color: "transparent"
        width: tipLabel.paintedWidth + units.gu(6)
        height: units.gu(1) + bottomEdge.tipHeight + bottomEdgeMarging
        visible: bottomEdgeEnabled
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: bottomEdge.top
        anchors.bottomMargin: -units.gu(0.5)
        z: 200

        Rectangle {
            id: bottomEdgeHint

            anchors {
                fill: parent
            }
            color: bottomEdgeBackgroundColor
            radius: units.gu(1)
            opacity: 0.6
        }

        Label {
            id: tipLabel

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: bottomEdge.tipHeight
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            color: bottomEdgeTitleColor
            fontSize: "medium"
            font.weight: Font.DemiBold
        }
    }

    Rectangle {
        id: tipFooter

        anchors.bottom: tip.bottom
        width: bottomEdge.width
        height: units.gu(0.5)
        color: bottomEdgeBackgroundColor
        z: 200
    }

    Rectangle {
        id: bottomEdge
        objectName: "bottomEdge"

        readonly property int tipHeight: units.gu(3)


        z: 1
        color: bottomEdgeBackgroundColor
        clip: true
        anchors {
            left: parent.left
            right: parent.right
        }
        height: page.height
        y: height

        visible: !page.isCollapsed
        state: "collapsed"
        states: [
            State {
                name: "collapsed"
                PropertyChanges {
                    target: bottomEdge
                    y: bottomEdge.height
                }
                PropertyChanges {
                    target: fakeHeader
                    y: -fakeHeader.height
                }
            },
            State {
                name: "expanded"
                PropertyChanges {
                    target: bottomEdge
                    y: __pageStartY
                }
                PropertyChanges {
                    target: fakeHeader
                    y: 0
                }
            },
            State {
                name: "floating"
                when: mouseArea.drag.active
                PropertyChanges {
                    target: shadow
                    opacity: 1.0
                    z: 200
                }
            }
        ]

        transitions: [
            Transition {
                to: "expanded"
                SequentialAnimation {
                    alwaysRunToEnd: true
                    ParallelAnimation {
                        SmoothedAnimation {
                            target: bottomEdge
                            property: "y"
                            duration: UbuntuAnimation.FastDuration
                            easing.type: Easing.Linear
                        }
                        SmoothedAnimation {
                            target: fakeHeader
                            property: "y"
                            duration: UbuntuAnimation.FastDuration
                            easing.type: Easing.Linear
                        }
                    }
                    SmoothedAnimation {
                        target: shaderSource
                        property: "anchors.topMargin"
                        to: - units.gu(4)
                        duration: UbuntuAnimation.FastDuration
                        easing.type: Easing.Linear
                    }
                    SmoothedAnimation {
                        target: shaderSource
                        property: "anchors.topMargin"
                        to: 0
                        duration: UbuntuAnimation.FastDuration
                        easing: UbuntuAnimation.StandardEasing
                    }
                    ScriptAction {
                        script: page.__pushPage()
                    }
                }
            },
            Transition {
                from: "expanded"
                to: "collapsed"
                SequentialAnimation {
                    alwaysRunToEnd: true

                    ScriptAction {
                        script: {
                            Qt.inputMethod.hide()
                            bottomEdgePage.active = false
                        }
                    }
                    ParallelAnimation {
                        SmoothedAnimation {
                            target: bottomEdge
                            property: "y"
                            duration: UbuntuAnimation.SlowDuration
                        }
                        SmoothedAnimation {
                            target: fakeHeader
                            property: "y"
                            duration: UbuntuAnimation.SlowDuration
                        }
                    }
                    ScriptAction {
                        script: {
                            // notify
                            page.bottomEdgeDismissed()
                            bottomEdgePage.active = true
                        }
                    }
                }
            },
            Transition {
                from: "floating"
                to: "collapsed"
                SmoothedAnimation {
                    target: bottomEdge
                    property: "y"
                    duration: UbuntuAnimation.FastDuration
                }
            }
        ]

        ShaderEffectSource {
            id: shaderSource
            sourceItem: bottomEdgePage
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: page.height + (fakeHeader.visible ? 0 : fakeHeader.height)

            Binding {
                target: bottomEdgePageLoaded ? shaderSource : null
                property: "anchors.topMargin"
                value:  bottomEdgePage && bottomEdgePage.flickable ? bottomEdgePage.flickable.contentY : 0
                when: !page.isReady
            }
        }
    } // Rectanlgle
}

