/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQml 2.0

QtObject {
    property int orientation: 1
    property var sequence: [1, 6, 3, 8]

    function rotateRight90() {
        var index = (sequence.indexOf(orientation) + 1) % sequence.length;
        orientation = sequence[index];
    }

    function add(a, b) {
        var index = (sequence.indexOf(a) + sequence.indexOf(b)) % sequence.length;
        return sequence[index];
    }
}
