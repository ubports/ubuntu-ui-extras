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

    property list<Action> toolActions: [
        Action { text: "ToolActions01" },
        Action { text: "ToolActions02" }
    ]

    property list<Action> filterActions: [
        Action { text: "FilterActions01" },
        Action { text: "FilterActions02" }
    ]

    ActionsBar {
        id: actionsBar
        toolActions: root.toolActions
    }

    UbuntuTestCase {
        id: actionsBarTestCase
        name: 'actionsBarTestCase'
        when: windowShown

        function init() {
            waitForRendering(actionsBar);
        }

        function cleanup() {
        }

        function test_actionsBarCheckColumnHeightWhitFilterActions() {
            actionsBar.filterActions = root.filterActions
            tryCompare(actionsBar, 'height', units.gu(20))
        }

        function test_actionsBarCheckColumnHeightWhitFilterActionsEmpty() {
            actionsBar.filterActions = []
            tryCompare(actionsBar, 'height', units.gu(6))
        }

        function test_actionsBarCheckFiltersBarNotVisibleWhitFilterActionsEmpty() {
            actionsBar.filterActions = []
            var filtersBar = findChild(actionsBar, 'filtersBar')
            tryCompare(filtersBar, 'visible', false)
        }
    }
}
