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

// A MouseArea meant to drag a corner/edge of a crop area.
MouseArea {
    id: cropDragArea

    /*!
    */
    signal dragged(real dx, real dy)
    /*!
    */
    signal dragStarted()
    /*!
    */
    signal dragCompleted()

    // Since we're usually moving this area with the mouse in response to
    // dragging, we don't need to capture the last x/y, just where it was
    // grabbed.
    property real grabX: -1
    /*!
    */
    property real grabY: -1

    onPressed: {
        dragStarted();

        grabX = mouse.x;
        grabY = mouse.y;
    }

    onReleased: {
        dragCompleted();
    }

    onPositionChanged: cropDragArea.dragged(mouse.x - grabX, mouse.y - grabY)
}
