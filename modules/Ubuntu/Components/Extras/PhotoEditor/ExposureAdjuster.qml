import QtQuick 2.0
import Ubuntu.Components 1.1
import Ubuntu.Components.Extras 0.1

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
                text: i18n.tr("Done")
                color: UbuntuColors.green
                enabled: adjuster.enabled
                onTriggered: {
                    targetImage.source = "";
                    confirm();
                }
            }
            Button {
                text: i18n.tr("Cancel")
                color: UbuntuColors.red
                enabled: adjuster.enabled
                onTriggered: {
                    targetImage.source = "";
                    cancel();
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
