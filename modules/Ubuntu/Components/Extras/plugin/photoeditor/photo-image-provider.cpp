/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Florian Boucault <florian.boucault@canonical.com>
 */

#include "photo-image-provider.h"

#include <QtCore/QFileInfo>
#include <QtGui/QImageReader>
#include <exiv2/exiv2.hpp>

const char* PhotoImageProvider::PROVIDER_ID = "photo";
const char* EXIF_ORIENTATION_KEY = "Exif.Image.Orientation";

PhotoImageProvider::PhotoImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

PhotoImageProvider::~PhotoImageProvider()
{
}

QImage PhotoImageProvider::requestImage(const QString& id,
                                        QSize* size, const QSize& requestedSize)
{
    QUrl url(id);
    QString filePath = url.path();

    QFileInfo fileInfo(filePath);
    QString original = fileInfo.path() + "/.original/" + fileInfo.fileName();
    if (QFileInfo::exists(original)) {
        Exiv2::Image::AutoPtr exivImage;
        try {
            exivImage = Exiv2::ImageFactory::open(filePath.toStdString());
            exivImage->readMetadata();
            Exiv2::ExifData& exifData = exivImage->exifData();
            if (exifData[EXIF_ORIENTATION_KEY].typeId() == Exiv2::signedLong) {
                exifData[EXIF_ORIENTATION_KEY] = (Exiv2::UShortValue)exifData[EXIF_ORIENTATION_KEY].toLong();
                exivImage->writeMetadata();
            }
        } catch (Exiv2::AnyError& e) {
        }
    }

    QImageReader reader(filePath);
    QSize fullSize = reader.size();
    QSize loadSize(fullSize);

    if (fullSize.isValid() && (requestedSize.width() > 0 || requestedSize.height() > 0)) {
        loadSize.scale(requestedSize, Qt::KeepAspectRatio);
        if (loadSize.width() > fullSize.width() || loadSize.height() > fullSize.height()) {
            loadSize = fullSize;
        }
    }

    if (loadSize != fullSize) {
        reader.setScaledSize(loadSize);
    }

    QImage image = reader.read();

    if (size != NULL) {
        *size = image.size();
    }

    return image;
}
