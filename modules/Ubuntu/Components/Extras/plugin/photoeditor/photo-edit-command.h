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

#ifndef GALLERY_PHOTO_EDIT_STATE_H_
#define GALLERY_PHOTO_EDIT_STATE_H_

// util
#include "orientation.h"

#include <QRectF>
#include <QVector4D>

enum EditType {
    EDIT_NONE = 0,
    EDIT_ROTATE = 1,
    EDIT_CROP = 2,
    EDIT_ENHANCE = 3,
    EDIT_COMPENSATE_EXPOSURE = 4
};

/*!
 * \brief The PhotoEditCommand class
 *
 * A single edit that will be applied to a photo.
 */
class PhotoEditCommand
{
public:
    EditType type;
    Orientation orientation;
    QRectF crop_rectangle;
    qreal exposureCompensation;
    /// The color balance parameters are stored here in the order:
    /// brightness (x), contrast(y), saturation(z), hue(w)
    QVector4D colorBalance_;

    PhotoEditCommand() :
        type(EDIT_NONE),
        orientation(ORIGINAL_ORIENTATION),
        crop_rectangle(),
        exposureCompensation(0.0) {
    }
};

#endif
