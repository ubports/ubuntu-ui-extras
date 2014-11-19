import QtQuick 2.2
import Ubuntu.Components 1.0
import Ubuntu.Components.Extras 0.1
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
    }

    property list<Action> toolActions: [
        Action {
            text: i18n.tr("Crop")
            iconSource: Qt.resolvedUrl("PhotoEditor/assets/edit_crop.png")
            onTriggered: {
                cropper.targetPhoto = "image://photo/" + photoData.path;
                pageBackground.opacity = 1.0;
            }
        },
        Action {
            text: i18n.tr("Rotate")
            iconSource: Qt.resolvedUrl("PhotoEditor/assets/edit_rotate.png")
            onTriggered: {
                photoData.rotateRight()
            }
        },
        Action {
            text: i18n.tr("Auto Enhance")
            iconSource: Qt.resolvedUrl("PhotoEditor/assets/edit_autocorrect.png")
            onTriggered: {
                photoData.autoEnhance();
            }
        },
        Action {
            text: i18n.tr("Adjust Exposure")
            iconSource: Qt.resolvedUrl("PhotoEditor/assets/edit_exposure.png")
            onTriggered: exposureSelector.start("image://photo/" + photoData.path);
        }
    ]

    function close() {
        stack.endEditingSession();
        editor.closed(editor.modified);
    }

    onPhotoChanged: {
        if (photo) {
            stack.startEditingSession(photo);
            photoData.path = stack.currentFile;
            image.source = "image://photo/" + photoData.path;
        }
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
        source: "image://photo/" + photoData.path
        fillMode: Image.PreserveAspectFit

        function reload() {
            image.asynchronous = false;
            image.source = "";
            image.asynchronous = true;
            image.source = "image://photo/" + photoData.path;
        }
    }

    Photo {
        id: photoData
        onDataChanged: image.reload()

        onEditFinished: {
            console.log("Edit finished")
            // If we are editing exposure we don't need to checkpoint at every
            // edit, and the exposure UI will checkpoint when the user confirms.
            if (exposureSelector.opacity > 0) exposureSelector.reload()
            else stack.checkpoint()
        }
    }

    // Note: this rectangle exists only because at the moment we are not using
    // the standard SDK PageStack in camera-app. Normally the cropper would
    // reside in a new page that gets pushed on and off the stack as needed.
    Rectangle {
        id: pageBackground

        color: "black"
        anchors.fill: parent
        opacity: 0.0

        Behavior on opacity { UbuntuNumberAnimation { } }

        CropInteractor {
            id: cropper
            anchors.fill: parent
            onCropped: {
                var qtRect = Qt.rect(rect.x, rect.y, rect.width, rect.height);
                photoData.crop(qtRect);
                targetPhoto = "";
                pageBackground.opacity = 0.0;
            }
            onCanceled: {
                targetPhoto = "";
                pageBackground.opacity = 0.0;
            }
        }
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
    }

    ActionsBar {
        id: actionsBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        visible: opacity > 0.0
        opacity: (exposureSelector.opacity == 0 && pageBackground.opacity == 0) ? 1.0 : 0.0

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

    ActivityIndicator {
        anchors.centerIn: parent
        running: photoData.busy
    }
}
