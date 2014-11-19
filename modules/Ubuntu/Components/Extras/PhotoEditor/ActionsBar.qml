import QtQuick 2.0
import Ubuntu.Components 1.1
import Ubuntu.Components.ListItems 1.0 as ListItem

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

            delegate: ListItem.Empty {
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
                    opacity: parent.enabled ? 1.0 : 0.5
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
