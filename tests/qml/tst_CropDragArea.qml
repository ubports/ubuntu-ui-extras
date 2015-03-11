/*
 * Copyright 2015 Canonical Ltd.
 *
 * This file is part of dialer-app.
 *
 * dialer-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * dialer-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.3
import QtTest 1.0
import Ubuntu.Test 0.1
import Ubuntu.Components 1.1

import '../../modules/Ubuntu/Components/Extras/PhotoEditor'

Item {
    id: root

    width: units.gu(40)
    height: units.gu(60)

    CropDragArea {
        id: cropDragArea
        anchors.fill: parent
    }

    SignalSpy {
        id: draggedSpy
        target: cropDragArea
        signalName: 'dragged'
    }

    SignalSpy {
        id: dragStartedSpy
        target: cropDragArea
        signalName: 'dragStarted'
    }

    SignalSpy {
        id: dragCompletedSpy
        target: cropDragArea
        signalName: 'dragCompleted'
    }

    UbuntuTestCase {
        id: cropDragAreaTestCase
        name: 'cropDragAreaTestCase'
        when: windowShown

        function init() {
            waitForRendering(cropDragArea);
        }

        function cleanup() {
            draggedSpy.clear()
            dragStartedSpy.clear()
            dragCompletedSpy.clear()
        }

        function test_cropDragAreaCheckDragExecution() {
            mousePress(cropDragArea, cropDragArea.width / 2, cropDragArea.height / 2)
            dragStartedSpy.wait()
            compare(dragStartedSpy.count, 1, 'dragStarted signal was not emitted.')

            mouseMove(cropDragArea, cropDragArea.width / 2, cropDragArea.height)
            draggedSpy.wait()
            compare(draggedSpy.count, 1, 'dragged signal was not emitted.')

            mouseRelease(cropDragArea, cropDragArea.width / 2, cropDragArea.height)
            dragCompletedSpy.wait()
            compare(dragCompletedSpy.count, 1, 'dragCompleted signal was not emitted.')
        }

        function test_cropDragAreaCheckClickExecution() {
            mouseClick(cropDragArea, cropDragArea.width / 2, cropDragArea.height / 2)
            dragStartedSpy.wait()
            compare(dragStartedSpy.count, 1, 'dragStarted signal was not emitted.')

            dragCompletedSpy.wait()
            compare(dragCompletedSpy.count, 1, 'dragCompleted signal was not emitted.')

            compare(draggedSpy.count, 0, 'dragged signal was emitted.')
        }
    }
}
