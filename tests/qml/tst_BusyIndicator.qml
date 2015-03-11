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

    BusyIndicator {
        id: busyIndicator
        text: "BusyIndicatorText"
    }

    UbuntuTestCase {
        id: busyIndicatorTestCase
        name: 'busyIndicatorTestCase'
        when: windowShown

        function init() {
            waitForRendering(busyIndicator);
        }

        function cleanup() {
        }

        function test_busyIndicatorCheckInvisibleWhenNotRunning() {
            busyIndicator.running = false
            tryCompare(busyIndicator, 'visible', false)
        }

        function test_busyIndicatorCheckIsVisibleWhenRunning() {
            busyIndicator.running = true
            tryCompare(busyIndicator, 'visible', true)
        }

        function test_busyIndicatorCheckLabelInvisibleWhenNotLongOperation() {
            busyIndicator.running = true
            busyIndicator.longOperation = false
            var l = findChild(busyIndicator, 'busyLabel')
            tryCompare(l, 'visible', false)
        }

        function test_busyIndicatorCheckLabelIsVisibleWhenLongOperation() {
            busyIndicator.running = true
            busyIndicator.longOperation = true
            var l = findChild(busyIndicator, 'busyLabel')
            tryCompare(l, 'visible', true)
        }

        function test_busyIndicatorCheckUbuntuShapeOpacityWhenNotLongOperation() {
            busyIndicator.running = true
            busyIndicator.longOperation = false
            var us = findChild(busyIndicator, 'busyUbuntuShape')
            tryCompare(us, 'opacity', 0)
        }

        function test_busyIndicatorCheckUbuntuShapeOpacityWhenLongOperation() {
            busyIndicator.running = true
            busyIndicator.longOperation = true 
            var us = findChild(busyIndicator, 'busyUbuntuShape')
            tryCompare(us, 'opacity', 0.75)
        }
    }
}
