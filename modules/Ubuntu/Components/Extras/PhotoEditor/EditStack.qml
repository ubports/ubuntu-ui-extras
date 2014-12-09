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

import QtQuick 2.0
import Ubuntu.Components 1.1
import Ubuntu.Components.Extras 0.1

Item {
    property PhotoData data
    property var items: []
    property int level: -1
    property string editingSessionPath
    property string currentFile
    property string originalFile
    property string pristineFile
    property bool modified: _revertedToPristine || level > 0

    property bool _revertedToPristine: false

    function startEditingSession(original) {
        var originalFileName = FileUtils.nameFromPath(original);
        var baseName = FileUtils.parentDirectory(original) +
                       "/.photo_editing." + originalFileName + ".";
        editingSessionPath = FileUtils.createTemporaryDirectory(baseName);
        if (editingSessionPath == "") return false;

        originalFile = original;
        currentFile = editingSessionPath + "/current";
        pristineFile = FileUtils.parentDirectory(originalFile) +
                       "/.original/" + originalFileName;

        FileUtils.copy(originalFile, currentFile)

        _revertedToPristine = false;
        items = [createSnapshot(0)];
        level = 0;
        return true;
    }

    function endEditingSession(saveIfModified) {
        if (saveIfModified &&
            (level > 0 || _revertedToPristine)) { // file modified

            // if we don't have a copy of the very first original, create one
            if (!FileUtils.exists(pristineFile)) {
                FileUtils.createDirectory(FileUtils.parentDirectory(pristineFile));
                FileUtils.copy(stack.originalFile, pristineFile);
            }

            FileUtils.copy(stack.currentFile, stack.originalFile); // actually save
        }

        FileUtils.removeDirectory(editingSessionPath, true); // clear editing cache
        editingSessionPath = originalFile = pristineFile = currentFile = "";
    }

    function createSnapshot(name) {
        var snapshotFile = editingSessionPath + "/edit." + name;
        FileUtils.copy(currentFile, snapshotFile);
        return snapshotFile;
    }

    function restoreSnapshot(name) {
        var snapshotFile = editingSessionPath + "/edit." + name;
        FileUtils.copy(snapshotFile, currentFile);
        data.refreshFromDisk();
    }

    function checkpoint() {
        level++;
        items = items.slice(0, level);
        items.push(createSnapshot(items.length));
    }

    property Action undoAction: Action {
            text: i18n.tr("Undo")
            iconName: "undo"
            enabled: items.length > 0 && level > 0
            onTriggered: restoreSnapshot(--level);
    }

    property Action redoAction: Action {
            text: i18n.tr("Redo")
            iconName: "redo"
            enabled: level < items.length - 1
            onTriggered: restoreSnapshot(++level);
    }

    property Action revertAction: Action {
            text: i18n.tr("Revert to Original")
            iconName: "revert"
            enabled: items.length > 0
            onTriggered: {
                if (!FileUtils.exists(pristineFile)) restoreSnapshot(0);
                else {
                    FileUtils.copy(pristineFile, currentFile);
                    data.refreshFromDisk();
                    _revertedToPristine = true;
                }

                items = items.slice(0, 1);
                level = 0;
            }
    }
}
