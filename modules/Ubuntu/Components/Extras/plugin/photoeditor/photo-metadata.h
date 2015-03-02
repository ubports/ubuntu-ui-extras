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

#ifndef GALLERY_PHOTO_METADATA_H_
#define GALLERY_PHOTO_METADATA_H_

// util
#include "orientation.h"

#include <QDateTime>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QSet>
#include <QTransform>
#include <QImage>

#include <exiv2/exiv2.hpp>

/*!
 * \brief The PhotoMetadata class
 */
class PhotoMetadata : public QObject
{
    Q_OBJECT

public:
    static PhotoMetadata* fromFile(const char* filepath);
    static PhotoMetadata* fromFile(const QFileInfo& file);

    QDateTime exposureTime() const;
    Orientation orientation() const;
    QTransform orientationTransform() const;
    OrientationCorrection orientationCorrection() const;

    void setOrientation(Orientation orientation);
    void setDateTimeDigitized(const QDateTime& digitized);

    void updateThumbnail(QImage image);
    void copyTo(PhotoMetadata* other) const;
    bool save() const;

private:
    PhotoMetadata(const char* filepath);
    
    Exiv2::Image::AutoPtr m_image;
    QSet<QString> m_keysPresent;
    QFileInfo m_fileSourceInfo;
};

#endif // GALLERY_PHOTO_METADATA_H_
