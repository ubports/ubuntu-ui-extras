import QtQuick 2.0
import QtTest 1.0
import Ubuntu.Components.Extras 0.1

TestCase {
    name: "PhotoImageProvider"
    height: 800
    width: 800

    function test_something() {
        console.log(image.paintedHeight, image.paintedWidth);
        compare(2, 2, "bleh");
    }

    Image {
        id: image
        source: "image://photo//home/nerochiaro/Pictures/fox.jpg"
        anchors.fill: parent
    }
}
