/*
 * Copyright (C) 2014 Canonical Ltd
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
import Ubuntu.Components.ListItems 1.3 as ListItem

Column {
    id: bar
    property list<Action> toolActions
    property list<Action> filterActions
    property bool enabled

    height: (filtersBar.visible) ? units.gu(20) : units.gu(6)

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        height: units.gu(6)

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.6
        }

        ListView {
            id: toolsBar
            anchors.fill: parent
            orientation: ListView.Horizontal
            model: toolActions

            delegate: AbstractButton {
                width: units.gu(8)
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                action: modelData
                enabled: bar.enabled

                Icon {
                    anchors.centerIn: parent
                    name: modelData.iconName
                    source: modelData.iconSource
                    width: units.gu(3)
                    height: units.gu(3)
                    opacity: modelData.enabled && parent.enabled ? 1.0 : 0.5
                }
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        height: units.gu(14)
        color: "black"

        ListView {
            id: filtersBar
            objectName: "filtersBar"
            visible: filterActions.length > 0

            orientation: ListView.Horizontal
            model: filterActions

            delegate: ListItem.Standard {
                width: parent.height
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                action: modelData
                iconFrame: false
                enabled: bar.enabled
            }
        }
    }
}
