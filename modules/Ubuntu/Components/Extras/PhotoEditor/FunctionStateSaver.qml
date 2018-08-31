/*
 * Copyright (C) 2015 Canonical Ltd
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

QtObject {
    property QtObject target
    property var values

    function qmltypeof(obj, className) {
        // className plus "(" is the class instance without modification
        // className plus "_QML" is the class instance with user-defined properties
        if (obj != null) {
            var str = obj.toString();
            return str.indexOf(className + "(") == 0 || str.indexOf(className + "_QML") == 0;
        } else {
            return false;
        }
    }

    function copy(destination, prop, value) {
        if (qmltypeof(value, "QMatrix4x4")) {
            destination[prop] = value.transposed().transposed();
        } else {
            destination[prop] = value;
        }
    }

    function save() {
        values = {};
        for (var prop in target) {
            if (typeof(target[prop]) !== "function") {
                copy(values, prop, target[prop]);
            }
        }
    }

    function restore() {
        for (var prop in values) {
            copy(target, prop, values[prop]);
        }
    }
}
