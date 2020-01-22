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
import Ubuntu.Components.Extras 0.2

// When the photo editor uses a proper PageStack this will switch back to being
// an Item as it will not need to cover what is below it.
Rectangle {
    id: adjuster
    color:"black"

    property alias exposure: exposureSelector.value
    property bool enabled

    signal confirm()
    signal cancel()

    Image {
        id: targetImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        cache: false
        sourceSize {
            width: targetImage.width
            height: targetImage.height
        }
    }

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: units.gu(2)
        spacing: units.gu(2)

        Slider {
            id: exposureSelector
            live: false
            minimumValue: -1.0
            maximumValue: +1.0
            value: 0.0
            enabled: adjuster.enabled

            anchors.left: parent.left
            anchors.right: parent.right
            height: units.gu(2)

            function formatValue(value) {
                return (Math.round(value * 100) / 100).toString()
            }
        }
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: units.gu(2)
            Button {
                text: i18n.dtr("ubuntu-ui-extras", "Cancel")
                enabled: adjuster.enabled
                onTriggered: {
                    targetImage.source = "";
                    cancel();
                }
            }
            Button {
                text: i18n.dtr("ubuntu-ui-extras", "Done")
                color: theme.palette.normal.positive
                enabled: adjuster.enabled
                onTriggered: {
                    targetImage.source = "";
                    confirm();
                }
            }
        }
    }

    function start(target) {
        targetImage.source = target;
        exposure = 0.0;
        opacity = 1.0;
    }

    function reload() {
        var path = targetImage.source;
        targetImage.asynchronous = false;
        targetImage.source = "";
        targetImage.asynchronous = true;
        targetImage.source = path;
    }
}
