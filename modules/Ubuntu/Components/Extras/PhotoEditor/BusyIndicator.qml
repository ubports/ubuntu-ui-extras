/*
 * Copyright (C) 2015 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
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
import Ubuntu.Components 1.3

Item {
    id: busy
    width: childrenRect.width
    height: childrenRect.height
    property alias text: label.text
    property alias running: spinner.running
    property bool longOperation: false

    visible: running

    UbuntuShape {
        id: busyUbuntuShape
        objectName: "busyUbuntuShape"
        backgroundColor: themes.palette.normal.background
        anchors.centerIn: parent
        width: parent.width + units.gu(4)
        height: parent.height + units.gu(4)
        opacity: longOperation ? 0.75 : 0
    }

    Column {
        id: busyColumn
        objectName: "busyColumn"
        anchors.centerIn: parent
        width: childrenRect.width
        spacing: units.gu(2)

        ActivityIndicator {
            id: spinner
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            id: label
            objectName: "busyLabel"
            anchors.horizontalCenter: parent.horizontalCenter
            horizontalAlignment: Text.AlignHCenter
            visible: longOperation
        }
    }
}
