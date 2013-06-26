import QtQuick 2.0
import Ubuntu.Components.Extras 0.1

Browser {
    height: 600
    width: 800

    Component.onCompleted: newTab("", true)
}
