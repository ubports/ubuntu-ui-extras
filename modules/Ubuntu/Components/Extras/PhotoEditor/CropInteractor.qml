/*
 * Copyright (C) 2012 Canonical Ltd
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
 *
 * Authors:
 * Charles Lindsay <chaz@yorba.org>
 * Lucas Beeler <lucas@yorba.org>
 */

import QtQuick 2.9
import Ubuntu.Components 1.3
import "GraphicsRoutines.js" as GraphicsRoutines

/*!
*/
Rectangle {
    id: cropInteractor
    objectName: "cropInteractor"

    color: "black"

    property alias targetPhoto: original.source

    property string matteColor: "black"
    property real matteOpacity: 0.6

    // Note: each element of the cropped rect will be in the range [0,1], since
    // in the UI we aren't using direct photo pixel values.
    signal cropped(variant rect)
    signal canceled()

    function computeRectSet() {
        var actualImage = Qt.rect(
            (original.width - original.paintedWidth) / 2.0,
            (original.height - original.paintedHeight) / 2.0,
            original.paintedWidth,
            original.paintedHeight
        );
        var photoPreview = GraphicsRoutines.fitRect(viewport, actualImage);

        var unfitCrop = Qt.rect(0, 0, photoPreview.width, photoPreview.height);
        var cropFrame = GraphicsRoutines.fitRect(viewport, unfitCrop);

        var photoExtent = Qt.rect(cropFrame.x, cropFrame.y,
                                  cropFrame.scaleFactor * photoPreview.width,
                                  cropFrame.scaleFactor * photoPreview.height);

        return {
            photoPreview: photoPreview,
            cropFrame: cropFrame,
            photoExtent: photoExtent,
            photoExtentScale: cropFrame.scaleFactor
        };
    }

    Item {
        id: viewport

        anchors.fill: parent
        anchors.margins: units.gu(6)
        z: 1
    }

    CropOverlay {
        id: overlay
        objectName: "cropOverlay"

        property real minSize: units.gu(4)

        anchors.fill: parent;
        visible: false;

        photo: original
        viewport: viewport

        matteColor: cropInteractor.matteColor
        matteOpacity: cropInteractor.matteOpacity

        z: 16

        onMatteRegionPressed: {
            cropInteractor.canceled();
        }

        onCropButtonPressed: {
            original.visible = false;
            overlay.visible = false;
            original.scale = 1.0;
            var r = overlay.getRelativeFrameRect()
            cropInteractor.cropped(overlay.getRelativeFrameRect());
        }
    }

    Image {
        id: original

        x: viewport.x
        y: viewport.y
        width: viewport.width
        height: viewport.height
        transformOrigin: Item.TopLeft
        fillMode: Image.PreserveAspectFit
        cache: false
        sourceSize {
            width: original.width
            height: original.height
        }

        onStatusChanged: {
            if (status == Image.Ready) {
                var rects = computeRectSet();

                overlay.initialFrameX = rects.cropFrame.x;
                overlay.initialFrameY = rects.cropFrame.y;
                overlay.initialFrameWidth = rects.cropFrame.width;
                overlay.initialFrameHeight = rects.cropFrame.height;

                overlay.resetFor(rects);
                overlay.visible = true;

                x = rects.photoExtent.x;
                y = rects.photoExtent.y;
                width = rects.photoPreview.width;
                height = rects.photoPreview.height;
                scale = rects.photoExtentScale;
            }
        }
    }
}
