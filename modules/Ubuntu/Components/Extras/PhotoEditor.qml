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
import Ubuntu.Components.Popups 1.3
import Ubuntu.Components.Extras 0.2
import "PhotoEditor"

Item {
    id: editor
    property string photo
    property bool modified: stack.modified

    signal closed(bool photoWasModified)

    property list<Action> actions
    actions: [stack.undoAction, stack.redoAction]

    EditStack {
        id: stack
        data: photoData
        actionsEnabled: !exposureSelector.visible && !cropper.visible && !photoData.busy
        onRevertRequested: PopupUtils.open(revertPromptComponent)
    }

    property list<Action> toolActions: [
        Action {
            objectName: "cropButton"
            text: i18n.dtr("ubuntu-ui-extras", "Crop")
            iconSource: Qt.resolvedUrl("PhotoEditor/assets/edit_crop.png")
            onTriggered: {
                photoData.isLongOperation = false;
                cropper.start("image://photo/" + photoData.path);
            }
        },
        Action {
            objectName: "rotateButton"
            text: i18n.dtr("ubuntu-ui-extras", "Rotate")
            iconSource: Qt.resolvedUrl("PhotoEditor/assets/edit_rotate_right.png")
            onTriggered: {
                photoData.isLongOperation = false;
                photoData.rotateRight()
            }
        },
        Action {
            objectName: "exposureButton"
            text: i18n.tr("Exposure")
            iconSource: Qt.resolvedUrl("PhotoEditor/assets/edit_exposure.png")
            onTriggered: {
                photoData.isLongOperation = false;
                exposureSelector.start("image://photo/" + photoData.path);
            }
        }
    ]

    function close(saveIfModified) {
        stack.endEditingSession(saveIfModified);
        editor.closed(editor.modified);
    }

    function open(photo) {
        editor.photo = photo;
        stack.startEditingSession(photo);
        photoData.path = stack.currentFile;
        image.source = "image://photo/" + photoData.path;
    }

    Rectangle {
        color: "black"
        anchors.fill: parent
    }

    Image {
        id: image
        anchors.fill: parent
        asynchronous: true
        cache: false
        source: photoData.path ? "image://photo/" + photoData.path : ""
        fillMode: Image.PreserveAspectFit
        sourceSize {
            width: image.width
            height: image.height
        }

        function reload() {
            image.asynchronous = false;
            image.source = "";
            image.asynchronous = true;
            image.source = "image://photo/" + photoData.path;
        }
    }

    PhotoData {
        id: photoData
        onDataChanged: image.reload()
        property bool isLongOperation: false

        onEditFinished: {
            console.log("Edit finished")
            // If we are editing exposure we don't need to checkpoint at every
            // edit, and the exposure UI will checkpoint when the user confirms.
            if (exposureSelector.opacity > 0) exposureSelector.reload()
            else stack.checkpoint()
        }
    }

    Loader {
        id: cropper

        anchors.fill: parent

        opacity: 0.0
        visible: opacity > 0
        Behavior on opacity { UbuntuNumberAnimation { } }

        Connections {
            target: cropper.item
            ignoreUnknownSignals: true
            onCropped: {
                var qtRect = Qt.rect(rect.x, rect.y, rect.width, rect.height);
                photoData.crop(qtRect);
                cropper.opacity = 0.0;
                cropper.source = ""
            }
            onCanceled: {
                cropper.opacity = 0.0;
                cropper.source = ""
            }
        }

        function start(target) {
            source = "PhotoEditor/CropInteractor.qml";
            item.targetPhoto = target;
        }

        onLoaded: opacity = 1.0
    }

    ExposureAdjuster {
        id: exposureSelector
        anchors.fill: parent
        opacity: 0.0
        enabled: !photoData.busy
        onExposureChanged: {
            // Restore the starting version of the image, otherwise we will
            // accumulate compensations over the previous ones.
            stack.restoreSnapshot(stack.level)
            photoData.exposureCompensation(exposure)
        }
        onConfirm: {
            stack.checkpoint();
            exposureSelector.opacity = 0.0
        }
        onCancel: {
            stack.restoreSnapshot(stack.level)
            exposureSelector.opacity = 0.0
        }
        visible: opacity > 0
    }

    ActionsBar {
        id: actionsBar
        objectName: "editorActionsBar"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        visible: opacity > 0.0
        opacity: (exposureSelector.opacity == 0 && cropper.opacity == 0) ? 1.0 : 0.0

        enabled: !photoData.busy
        toolActions: {
            // This is necessary because QML does not let us declare a list with
            // mixed component declarations and identifiers, like this:
            // property list<Action> foo: { Action{}, someOtherAction }
            var list = [];
            for (var i = 0; i < editor.toolActions.length; i++)
                list.push(editor.toolActions[i]);
            list.push(stack.revertAction);
            return list;
        }

        Behavior on opacity { UbuntuNumberAnimation {} }
    }

    Component {
        id: revertPromptComponent
        Dialog {
            id: revertPrompt
            objectName: "revertPromptDialog"
            title: i18n.dtr("ubuntu-ui-extras", "Revert to original")
            text: i18n.dtr("ubuntu-ui-extras", "This will undo all edits, including those from previous sessions.")

            Row {
                id: row
                width: parent.width
                spacing: units.gu(1)
                Button {
                    objectName: "cancelRevertButton"
                    width: parent.width/2
                    text: i18n.dtr("ubuntu-ui-extras", "Cancel")
                    onClicked: PopupUtils.close(revertPrompt)
                }
                Button {
                    objectName: "confirmRevertButton"
                    width: parent.width/2
                    text: i18n.dtr("ubuntu-ui-extras", "Revert Photo")
                    color: theme.palette.normal.negative
                    onClicked: {
                        PopupUtils.close(revertPrompt)
                        stack.revertToPristine()
                    }
                }
            }
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        text: i18n.dtr("ubuntu-ui-extras", "Enhancing photo...")
        running: photoData.busy
        longOperation: photoData.isLongOperation
    }
}
