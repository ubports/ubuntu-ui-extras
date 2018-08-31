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
import QtHalide 0.1

HalideFunction {
    name: "transform"

    property bool rotationOnly: true
    property HalideImage input
    property matrix4x4 transform: Qt.matrix4x4(1, 0, 0, 0,
                                               0, 1, 0, 0,
                                               0, 0, 1, 0,
                                               0, 0, 0, 1)
    property matrix4x4 size: Qt.matrix4x4(1, 0, 0, 0,
                                          0, 1, 0, 0,
                                          0, 0, 1, 0,
                                          0, 0, 0, 1)

    function rotateRight90() {
        var angle = Math.PI/2;
        var resizing = Qt.matrix4x4(Math.cos(angle), Math.sin(angle), 0, 0,
                                    Math.sin(angle), Math.cos(angle), 0, 0,
                                    0,               0,               1, 0,
                                    0,               0,               0, 1);

        var originalSize = Qt.vector3d(1, 1, 1);
        var inputSize = size.times(originalSize);
        var outputSize = resizing.times(inputSize);

        var centering = Qt.matrix4x4(1, 0, (outputSize.x - inputSize.x)/2, 0,
                                     0, 1, (outputSize.y - inputSize.y)/2, 0,
                                     0, 0, 1,                              0,
                                     0, 0, 0,                              1);
        var translation = Qt.matrix4x4(1, 0, inputSize.x/2.0, 0,
                                       0, 1, inputSize.y/2.0, 0,
                                       0, 0, 1,               0,
                                       0, 0, 0,               1);
        var rotation = Qt.matrix4x4(Math.cos(angle), -Math.sin(angle), 0, 0,
                                    Math.sin(angle), Math.cos(angle),  0, 0,
                                    0, 0,                              1, 0,
                                    0, 0,                              0, 1);
        transform = transform.times(centering.times(translation.times(rotation.times(translation.inverted()))).inverted());
        size = resizing.times(size);
    }

    function crop(startX, startY, cropWidth, cropHeight) {
        rotationOnly = false;
        var resizing = Qt.matrix4x4(cropWidth, 0,          0, 0,
                                    0,         cropHeight, 0, 0,
                                    0,         0,          1, 0,
                                    0,         0,          0, 1);

        var originalSize = Qt.vector3d(1, 1, 1);
        var inputSize = size.times(originalSize);
        var translation = Qt.matrix4x4(1, 0, startX*inputSize.x, 0,
                                       0, 1, startY*inputSize.y, 0,
                                       0, 0, 1,                  0,
                                       0, 0, 0,                  1);
        transform = transform.times(translation);
        size = resizing.times(size);
    }
}
