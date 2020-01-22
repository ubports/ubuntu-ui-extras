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

/* A CropOverlay is a semi-transparent surface that floats over the photo. It
 * serves two purposes. First, it provides visual cueing as to what region of
 * the photo's surface will be preserved when the crop operation is applied.
 * The preserved region is the region that falls inside of the CropOverlay's
 * frame. Second, the CropOverlay allows the user to manipulate the
 * geometry of the crop frame, to chage its location, width, and height. The
 * geometry of the crop frame is reinforced by a key visual cue: the region of
 * the photo outside of the crop frame is drawn with a semi-transparent, smoked
 * matte on top of it. This matte surrounds the crop frame.
 */
Item {
    id: cropOverlay

    // public properties
    /*!
    */
    property Item viewport
    /*!
    */
    property Item photo
    /*!
    */
    property string matteColor: "red"
    /*!
    */
    property real matteOpacity: 0.85
    /*!
    */
    property int initialFrameX: -1
    /*!
    */
    property int initialFrameY: -1
    /*!
    */
    property int initialFrameWidth: -1
    /*!
    */
    property int initialFrameHeight: -1

    // private properties -- Frame Fit Animation parameters
    property real interpolationFactor: 1.0
    /*!
    */
    property variant startFrame
    /*!
    */
    property variant endFrame
    /*!
    */
    property variant startPhoto
    /*!
    */
    property real referencePhotoWidth: -1
    /*!
    */
    property real referencePhotoHeight: -1
    /*!
    */
    property real endPhotoX
    /*!
    */
    property real endPhotoY
    /*!
    */
    property real endPhotoWidth
    /*!
    */
    property real endPhotoHeight

    /*!
    */
    signal userAlteredFrame()
    /*!
    */
    signal runFrameFitAnimation()
    /*!
    */
    signal matteRegionPressed()
    /*!
    */
    signal cropButtonPressed()

    /*!
    */
    function resetFor(rectSet) {
        if (initialFrameX != -1 && initialFrameY != -1 && initialFrameWidth != -1 &&
                initialFrameHeight != -1) {
            frame.x = rectSet.cropFrame.x;
            frame.y = rectSet.cropFrame.y;
            frame.width = rectSet.cropFrame.width;
            frame.height = rectSet.cropFrame.height;
            photoExtent.x = rectSet.photoExtent.x;
            photoExtent.y = rectSet.photoExtent.y;
            photoExtent.width = rectSet.photoExtent.width;
            photoExtent.height = rectSet.photoExtent.height;
            referencePhotoWidth = rectSet.photoPreview.width;
            referencePhotoHeight = rectSet.photoPreview.height;
        }
    }

    /* Return the (x, y) position and the width and height of the viewport
    */
    function getViewportExtentRect() {
        return GraphicsRoutines.cloneRect(viewport);
    }

    /* Return the (x, y) position and the width and height of the photoExtent.
     * The photoExtent is the on-screen region that holds the original photo
     * preview.
     */
    function getPhotoExtentRect() {
        return GraphicsRoutines.cloneRect(photoExtent);
    }

    /*!
    */
    function getRelativeFrameRect() {
        return GraphicsRoutines.getRelativeRect(frame.getExtentRect(),
                                                getPhotoExtentRect());
    }

    anchors.fill: parent

    Item {
        id: photoExtent

        property real panStartX
        property real panStartY

        function startPan() {
            panStartX = x;
            panStartY = y;
        }

        // 'deltaX' and 'deltaY' are offsets relative to the pan start point
        function updatePan(deltaX, deltaY) {
            var newX = panStartX + deltaX;
            var newY = panStartY + deltaY;

            x = GraphicsRoutines.clamp(newX, frame.x + frame.width -
                                       photoExtent.width, frame.x);
            y = GraphicsRoutines.clamp(newY, frame.y + frame.height -
                                       photoExtent.height, frame.y);
        }

        function stopPan() {
        }

        x: initialFrameX
        y: initialFrameY
        width: initialFrameWidth
        height: initialFrameHeight
        z: 1

        onXChanged: {
            if (photo)
                photo.x = x;
        }

        onYChanged: {
            if (photo)
                photo.y = y;
        }

        onWidthChanged: {
            if (photo && referencePhotoWidth > 0)
                photo.scale = width / referencePhotoWidth;
        }

        onHeightChanged: {
            if (photo && referencePhotoHeight > 0)
                photo.scale = height / referencePhotoHeight;
        }
    }

    //
    // The following four Rectangles are used to "matte out" the area of the photo
    // preview that falls outside the frame. This "matting out" visual cue is
    // accomplished by darkening the matted-out area with a translucent, smoked
    // overlay.
    //
    Rectangle {
        id: leftMatte

        color: cropOverlay.matteColor
        opacity: cropOverlay.matteOpacity

        anchors.top: topMatte.bottom
        anchors.bottom: frame.bottom
        anchors.left: parent.left
        anchors.right: frame.left

        MouseArea {
            anchors.fill: parent;

            onPressed: cropOverlay.matteRegionPressed();
        }
    }

    Rectangle {
        id: topMatte

        color: cropOverlay.matteColor
        opacity: cropOverlay.matteOpacity

        anchors.top: parent.top
        anchors.bottom: frame.top
        anchors.left: parent.left
        anchors.right: parent.right

        MouseArea {
            anchors.fill: parent;

            onPressed: cropOverlay.matteRegionPressed();
        }
    }

    Rectangle {
        id: rightMatte

        color: cropOverlay.matteColor
        opacity: cropOverlay.matteOpacity

        anchors.top: topMatte.bottom
        anchors.bottom: bottomMatte.top
        anchors.left: frame.right
        anchors.right: parent.right

        MouseArea {
            anchors.fill: parent;

            onPressed: cropOverlay.matteRegionPressed();
        }
    }

    Rectangle {
        id: bottomMatte

        color: cropOverlay.matteColor
        opacity: cropOverlay.matteOpacity

        anchors.top: frame.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        MouseArea {
            anchors.fill: parent;

            onPressed: cropOverlay.matteRegionPressed();
        }
    }

    //
    // The frame is a grey rectangle with associated drag corners that
    // frames the region of the photo that will remain when the crop operation is
    // applied.
    //
    // NB: the frame can be in two states, although the QML state mechanism
    //     isn't sufficiently expressive to describe them. The frame can be
    //     in the FIT state, in which case it is optimally fit inside the
    //     frame constraint region (see getFrameConstraintRect( ) above for
    //     a description of the frame constraint region). Or, the frame can
    //     be in the USER state. In the user state, the user has the mouse button
    //     held down and is actively performing a drag operation to change the
    //     geometry of the frame.
    //
    Rectangle {
        id: frame

        signal resizedX(bool left, real dx)
        signal resizedY(bool top, real dy)

        property variant dragStartRect

        function getExtentRect() {
            var result = { };

            result.x = x;
            result.y = y;
            result.width = width;
            result.height = height;

            return result;
        }

        x: cropOverlay.initialFrameX
        y: cropOverlay.initialFrameY
        width: cropOverlay.initialFrameWidth
        height: cropOverlay.initialFrameHeight

        color: "transparent"

        border.width: units.gu(0.2)
        border.color: "#19B6EE"

        MouseArea {
            id: panArea

            property int dragStartX;
            property int dragStartY;

            anchors.fill: parent
            anchors.margins: 2

            onPressed: {
                dragStartX = mouse.x;
                dragStartY = mouse.y;

                photoExtent.startPan();
            }

            onReleased: {
                photoExtent.stopPan();
            }

            onPositionChanged: {
                photoExtent.updatePan(mouse.x - dragStartX, mouse.y - dragStartY);
            }
        }

        Button {
            objectName: "centerCropIcon"
            anchors.centerIn: parent
            text: i18n.dtr("ubuntu-ui-extras", "Crop")
            color: frame.border.color
            opacity: 0.9
            onClicked: cropOverlay.cropButtonPressed()
        }

        // Left drag bar.
        CropDragArea {
            x: -units.gu(2)
            width: units.gu(3)
            anchors.verticalCenter: parent.center
            height: parent.height - units.gu(2)

            onDragged: {
                frame.resizedX(true, dx);
                frame.updateOnAltered(false);
            }

            onDragStarted: frame.dragStartRect = frame.getExtentRect();
            onDragCompleted: frame.updateOnAltered(true);
        }

        // Top drag bar.
        CropDragArea {
            y: -units.gu(2)
            height: units.gu(3)
            anchors.horizontalCenter: parent.center
            width: parent.width - units.gu(2)

            onDragged: {
                frame.resizedY(true, dy);
                frame.updateOnAltered(false);
            }

            onDragStarted: frame.dragStartRect = frame.getExtentRect();
            onDragCompleted: frame.updateOnAltered(true);
        }

        // Right drag bar.
        CropDragArea {
            x: parent.width - units.gu(1)
            width: units.gu(3)
            anchors.verticalCenter: parent.center
            height: parent.height - units.gu(2)

            onDragged: {
                frame.resizedX(false, dx);
                frame.updateOnAltered(false);
            }

            onDragStarted: frame.dragStartRect = frame.getExtentRect();
            onDragCompleted: frame.updateOnAltered(true);
        }

        // Bottom drag bar.
        CropDragArea {
            y: parent.height - units.gu(1)
            height: units.gu(3)
            anchors.horizontalCenter: parent.center
            width: parent.width - units.gu(2)

            onDragged: {
                frame.resizedY(false, dy);
                frame.updateOnAltered(false);
            }

            onDragStarted: frame.dragStartRect = frame.getExtentRect();
            onDragCompleted: frame.updateOnAltered(true);
        }

        // Top-left corner.
        CropCorner {
            objectName: "topLeftCropCorner"
            isLeft: true
            isTop: true

            onDragged: {
                frame.resizedX(isLeft, dx);
                frame.resizedY(isTop, dy);
                frame.updateOnAltered(false);
            }

            onDragStarted: frame.dragStartRect = frame.getExtentRect();
            onDragCompleted: frame.updateOnAltered(true);
        }

        // Top-right corner.
        CropCorner {
            objectName: "topRightCropCorner"
            isLeft: false
            isTop: true

            onDragged: {
                frame.resizedX(isLeft, dx);
                frame.resizedY(isTop, dy);
                frame.updateOnAltered(false);
            }

            onDragStarted: frame.dragStartRect = frame.getExtentRect();
            onDragCompleted: frame.updateOnAltered(true);
        }

        // Bottom-left corner.
        CropCorner {
            objectName: "bottonLeftCropCorner"
            isLeft: true
            isTop: false

            onDragged: {
                frame.resizedX(isLeft, dx);
                frame.resizedY(isTop, dy);
                frame.updateOnAltered(false);
            }

            onDragStarted: frame.dragStartRect = frame.getExtentRect();
            onDragCompleted: frame.updateOnAltered(true);
        }

        // Bottom-right corner.
        CropCorner {
            id: bottomRightCrop
            objectName: "bottomRightCropCorner"
            isLeft: false
            isTop: false

            onDragged: {
                frame.resizedX(isLeft, dx);
                frame.resizedY(isTop, dy);
                frame.updateOnAltered(false);
            }

            onDragStarted: frame.dragStartRect = frame.getExtentRect();
            onDragCompleted: frame.updateOnAltered(true);
        }

        // This handles resizing in both dimensions.  first is whether we're
        // resizing the "first" edge, e.g. left or top (in which case we
        // adjust both position and span) vs. right or bottom (where we just
        // adjust the span).  position should be either "x" or "y", and span
        // is either "width" or "height".  This is a little complicated, and
        // coule probably be optimized with a little more thought.
        function resizeFrame(first, delta, position, span) {
            var constraintRegion = cropOverlay.getPhotoExtentRect();

            if (first) {
                // Left/top side.
                if (frame[position] + delta < constraintRegion[position])
                    delta = constraintRegion[position] - frame[position]

                if (frame[span] - delta < minSize)
                    delta = frame[span] - minSize;

                frame[position] += delta;
                frame[span] -= delta;
            } else {
                // Right/bottom side.
                if (frame[span] + delta < minSize)
                    delta = minSize - frame[span];

                if ((frame[position] + frame[span] + delta) >
                        (constraintRegion[position] + constraintRegion[span]))
                    delta = constraintRegion[position] + constraintRegion[span] -
                            frame[position] - frame[span];

                frame[span] += delta;
            }
        }

        onResizedX: resizeFrame(left, dx, "x", "width")
        onResizedY: resizeFrame(top, dy, "y", "height")

        function updateOnAltered(finalUpdate) {
            var start = frame.dragStartRect;
            var end = frame.getExtentRect();
            if (!GraphicsRoutines.areEqual(end, start)) {
                if (finalUpdate ||
                    (end.width * end.height >= start.width * start.height)) {
                    cropOverlay.userAlteredFrame();
                    cropOverlay.runFrameFitAnimation();
                }
            }
        }
    }

    /* Invoked when the user has changed the geometry of the frame by dragging
     * one of its corners or edges. Expressed in terms of the states of the
     * frame described above, the userAlteredFrame signal is fired
     * when the user stops dragging. This triggers a change of the frame
     * from the USER state to the FIT state
     */
    onUserAlteredFrame: {
        // since the geometry of the frame in the FIT state depends on both
        // how the user resized the frame when it was in the USER state as well
        // as the size of the frame constraint region, we have to recompute the
        // geometry of of the frame for the FIT state every time.

        startFrame = GraphicsRoutines.cloneRect(frame);

        endFrame = GraphicsRoutines.fitRect(getViewportExtentRect(),
                                            frame.getExtentRect());

        startPhoto = GraphicsRoutines.cloneRect(photoExtent);

        var frameRelativeToPhoto = getRelativeFrameRect();
        var scaleFactor = endFrame.width / frame.width;

        endPhotoWidth = photoExtent.width * scaleFactor;
        endPhotoHeight = photoExtent.height * scaleFactor;
        endPhotoX = endFrame.x - (frameRelativeToPhoto.x * endPhotoWidth);
        endPhotoY = endFrame.y - (frameRelativeToPhoto.y * endPhotoHeight)

        photo.transformOrigin = Item.TopLeft;
    }

    onRunFrameFitAnimation: NumberAnimation { target: cropOverlay;
        property: "interpolationFactor"; from: 0.0; to: 1.0 }

    onInterpolationFactorChanged: {
        var endPhotoRect = { };
        endPhotoRect.x = endPhotoX;
        endPhotoRect.y = endPhotoY;
        endPhotoRect.width = endPhotoWidth;
        endPhotoRect.height = endPhotoHeight;

        var interpolatedRect = GraphicsRoutines.interpolateRect(startFrame,
                                                                endFrame, interpolationFactor);
        GraphicsRoutines.sizeToRect(interpolatedRect, frame);

        interpolatedRect = GraphicsRoutines.interpolateRect(startPhoto,
                                                            endPhotoRect, interpolationFactor);
        GraphicsRoutines.sizeToRect(interpolatedRect, photoExtent);
    }
}
