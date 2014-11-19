/*
 * Copyright (C) 2011 Canonical Ltd
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
 * Lucas Beeler <lucas@yorba.org>
 */

#ifndef GALLERY_ORIENTATION_H_
#define GALLERY_ORIENTATION_H_

#include <QTransform>

enum Orientation {
    ORIGINAL_ORIENTATION = 0,
    MIN_ORIENTATION = 1,
    TOP_LEFT_ORIGIN = 1,
    TOP_RIGHT_ORIGIN = 2,
    BOTTOM_RIGHT_ORIGIN = 3,
    BOTTOM_LEFT_ORIGIN = 4,
    LEFT_TOP_ORIGIN = 5,
    RIGHT_TOP_ORIGIN = 6,
    RIGHT_BOTTOM_ORIGIN = 7,
    LEFT_BOTTOM_ORIGIN = 8,
    MAX_ORIENTATION = 8
};

/*!
 * \brief The OrientationCorrection struct
 */
class OrientationCorrection
{
public:
    static OrientationCorrection fromOrientation(Orientation o);
    static OrientationCorrection identity();
    static Orientation rotateOrientation(Orientation orientation, bool left);

    QTransform toTransform() const;

    bool isFlippedFrom(const OrientationCorrection& other) const;
    int getNormalizedRotationDifference(const OrientationCorrection& other) const;

private:
    OrientationCorrection(double rotation_angle, double horizontal_scale_factor)
        : m_rotationAngle(rotation_angle),
          m_horizontalScaleFactor(horizontal_scale_factor) { }

    const double m_rotationAngle;
    const double m_horizontalScaleFactor;
};


#endif // GALLERY_ORIENTATION_H_
