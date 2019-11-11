/*
 * Copyright (C) 2018
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

Item {
    id: item
    property alias padding: column.padding
    property alias model: tumb.model
    property alias currentIndex: tumb.currentIndex
    property int stepOnHold: 6
    property real fontPointSize: units.fs("x-large")

    height: column.implicitHeight
    width: column.implicitWidth

    function formatText(modelData) {
        return modelData.toString().length < 2 ? "0" + modelData : modelData;
    }

    Component {
        id: delegateComponent

        Label {
            text: formatText(modelData)
            opacity: 0.9 - Math.abs(Tumbler.displacement) / (Tumbler.tumbler.visibleItemCount / 2)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: fontPointSize
        }
    }

    Column {
        id: column
        padding: 0

        Item {
            width: tumb.width
            height: more.height
            Icon {
                id: more
                source: "qrc:/images/go-up.svg"
                anchors.centerIn: parent

                onPressed: {
                    var i = tumb.currentIndex + 1;
                    tumb.currentIndex = i < tumb.count ? i : 0;
                }
                onPressAndHold: {
                    var i = tumb.currentIndex + stepOnHold - 1;
                    tumb.currentIndex = i < tumb.count ? i : i - tumb.count;
                }

                width: units.gu(5)
                height: width
                opacity: 0.7
            }
        }

        Tumbler {
            id: tumb
            model: 24
            delegate: delegateComponent
            visibleItemCount: 1
            height: fontPointSize * 2
            width: units.gu(5)
        }

        Item {
            width: tumb.width
            height: less.height
            Icon {
                id: less
                source: "qrc:/images/go-down.svg"
                anchors.centerIn: parent

                onPressed: {
                    tumb.currentIndex = tumb.currentIndex > 0 ? tumb.currentIndex - 1 : tumb.count - 1;
                }
                onPressAndHold: {
                    var i = tumb.currentIndex - stepOnHold + 1;
                    tumb.currentIndex = i >= 0 ? i : i + tumb.count;
                }

                width: units.gu(5)
                height: width
                opacity: 0.7
            }
        }
    }

}
