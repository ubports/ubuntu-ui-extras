/*
 * Copyright 2013 Canonical Ltd.
 *
 * This file is part of ubuntu-ui-extras.
 *
 * ubuntu-ui-extras is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * ubuntu-ui-extras is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtTest 1.0
import Ubuntu.Components.Extras 0.1

TestCase {
    name: "ExampleDelegate"

    function test_has_three_items() {
        compare(example.model.count, 3)
    }

    Example {
        id: example
    }
}
