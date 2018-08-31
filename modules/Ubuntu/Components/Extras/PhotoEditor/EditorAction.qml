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

import QtQuick 2.3
import Ubuntu.Components 1.1
import QtHalide 0.1

Action {
    id: editorAction

    property HalideFunction halideFunction
    property HalideTransform halideTransform
    property bool manualCommit: false
    property var savedState

    onTriggered: {
        halideTransform.live = false;
        savedState = internal.snapshotFunctionState(halideFunction);
        execute();
        if (!manualCommit) {
            commit();
        }
    }

    function commit() {
        internal.pushUndoState(savedState);
        internal.clearRedoStates();
        halideFunction.enabled = true;
        halideTransform.live = true;
    }

    function execute() {
        // to be overridden by subclasses
    }
}
