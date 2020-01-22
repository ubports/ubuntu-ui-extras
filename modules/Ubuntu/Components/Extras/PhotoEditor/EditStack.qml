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

import QtQuick 2.9
import Ubuntu.Components 1.3
import Ubuntu.Components.Extras 0.2

Item {
    property PhotoData data
    property bool actionsEnabled: true
    property var items: []
    property int level: 0
    property string editingSessionPath
    property string currentFile
    property string originalFile
    property string pristineFile
    property bool modified: level > 0 || _revertedInThisSession

    property bool _revertedInThisSession
    property bool _pristineFileExists

    signal revertRequested

    function startEditingSession(original) {
        var originalFileName = FileUtils.nameFromPath(original);
        var baseName = FileUtils.parentDirectory(original) +
                       "/.photo_editing." + originalFileName + ".";
        editingSessionPath = FileUtils.createTemporaryDirectory(baseName);
        if (editingSessionPath == "") return false;

        originalFile = original;
        currentFile = editingSessionPath + "/current";

        pristineFile = FileUtils.parentDirectory(original) +
                       "/.original/" + originalFileName
        _revertedInThisSession = false;
        _pristineFileExists = FileUtils.exists(pristineFile)

        FileUtils.copy(originalFile, currentFile)

        items = [createSnapshot(0)];
        level = 0;
        return true;
    }

    function endEditingSession(saveIfModified) {
        if (saveIfModified && modified) { // file modified
            // if we don't have a copy of the very first original, create one
            if (!_pristineFileExists) {
                FileUtils.createDirectory(FileUtils.parentDirectory(pristineFile));
                FileUtils.copy(originalFile, pristineFile);
            } else {
                // if we reverted to original (and made no other changes)
                // we don't need to keep the pristine copy around
                if (_revertedInThisSession && level <= 0) {
                    FileUtils.remove(pristineFile);
                }
            }

            FileUtils.copy(currentFile, originalFile); // actually save
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

    function revertToPristine() {
        if (!FileUtils.exists(pristineFile)) {
            restoreSnapshot(0);
            items = items.slice(0, 1);
            level = 0;
        } else {
            FileUtils.copy(pristineFile, currentFile);
            data.refreshFromDisk();
            items = [];
            checkpoint();
            level = 0;
            _revertedInThisSession = true;
        }
    }

    property Action undoAction: Action {
            text: i18n.dtr("ubuntu-ui-extras", "Undo")
            iconName: "undo"
            enabled: items.length > 0 && level > 0 && actionsEnabled
            onTriggered: restoreSnapshot(--level);
    }

    property Action redoAction: Action {
            text: i18n.dtr("ubuntu-ui-extras", "Redo")
            iconName: "redo"
            enabled: level < items.length - 1 && actionsEnabled
            onTriggered: restoreSnapshot(++level);
    }

    property Action revertAction: Action {
            text: i18n.dtr("ubuntu-ui-extras", "Revert to original")
            iconSource: Qt.resolvedUrl("assets/edit_revert.png")
            enabled: actionsEnabled &&
                     (level > 0 || (!_revertedInThisSession && _pristineFileExists))
            onTriggered: revertRequested()
    }
}
