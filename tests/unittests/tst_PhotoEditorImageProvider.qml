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

import QtQuick 2.3
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
