/*
 * Copyright (C) 2012 Canonical Ltd
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
 *
 * Authors:
 * Charles Lindsay <chaz@yorba.org>
 */

import QtQuick 2.9
import Ubuntu.Components 1.3

// A corner of a CropFrame.
Item {
    id: cropCorner

    /*!
    */
    signal dragged(real dx, real dy)
    /*!
    */
    signal dragStarted()
    /*!
    */
    signal dragCompleted()

    /*!
    */
    property bool isLeft: true
    /*!
    */
    property bool isTop: true

    x: isLeft ? -(width/2) : parent.width - (width/2)
    y: isTop ? -(width/2) : parent.height - (width/2)
    width: handle.width
    height: handle.height

    Image {
        id: handle
        anchors.centerIn: parent
        source: Qt.resolvedUrl("assets/crop-handle.png")
    }

    CropDragArea {
        anchors.centerIn: parent
        width: handle.width + units.gu(2)
        height: handle.height + units.gu(2)

        onDragged: cropCorner.dragged(dx, dy)

        onDragStarted: cropCorner.dragStarted()

        onDragCompleted: cropCorner.dragCompleted()
    }
}
