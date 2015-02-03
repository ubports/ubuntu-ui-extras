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

import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.Popups 1.0

Dialog {
    id: busy
    width: childrenRect.width
    height: childrenRect.height
    property alias text: label.text

    Column {
        anchors.centerIn: parent
        width: childrenRect.width
        spacing: units.gu(2)

        ActivityIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            running: true
        }

        Label {
            id: label
            anchors.horizontalCenter: parent.horizontalCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
