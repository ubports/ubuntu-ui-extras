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
 */

#include "photo-metadata.h"

#include <QBuffer>
#include <QtConcurrent/QtConcurrent>

namespace {
const char* EXIF_ORIENTATION_KEY = "Exif.Image.Orientation";

} // anonymous namespace

namespace PhotoEditor {

PhotoMetadata::PhotoMetadata()
{
    connect(&m_saveWatcher, SIGNAL(finished()), this, SIGNAL(savingChanged()));
}

QString PhotoMetadata::fileName() const
{
    return m_fileName;
}

void PhotoMetadata::setFileName(QString fileName)
{
    if (fileName != m_fileName) {
        m_fileName = fileName;
        try {
            m_image = Exiv2::ImageFactory::open(fileName.toStdString());
            if (!m_image->good()) {
                qDebug("Invalid image metadata in %s", fileName.toLocal8Bit().constData());
            }
            m_image->readMetadata();
        } catch (Exiv2::AnyError& e) {
            qDebug("Error loading image metadata: %s", e.what());
        }
        Q_EMIT fileNameChanged();
    }
}

int PhotoMetadata::orientation() const
{
    Exiv2::ExifData& exif_data = m_image->exifData();

    if (exif_data.empty())
        return TOP_LEFT_ORIGIN;

    long orientation_code = exif_data[EXIF_ORIENTATION_KEY].toLong();
    if (orientation_code < MIN_ORIENTATION || orientation_code > MAX_ORIENTATION)
        return TOP_LEFT_ORIGIN;

    return static_cast<Orientation>(orientation_code);
}

void PhotoMetadata::setOrientation(int orientation)
{
    Exiv2::ExifData& exif_data = m_image->exifData();
    exif_data[EXIF_ORIENTATION_KEY] = (Exiv2::UShortValue)orientation;
}

bool PhotoMetadata::saving() const
{
    return m_saveWatcher.isRunning();
}

void PhotoMetadata::updateThumbnail(QImage image)
{
    QImage scaled = image.scaled(256, 256,  Qt::KeepAspectRatio);
    QBuffer jpeg;
    jpeg.open(QIODevice::WriteOnly);
    scaled.save(&jpeg, "jpeg");
    Exiv2::ExifThumb thumb(m_image->exifData());
    thumb.setJpegThumbnail((Exiv2::byte*) jpeg.data().constData(), jpeg.size());
}

void PhotoMetadata::saveImage(QImage image)
{
    QFuture<void> future = QtConcurrent::run(this, &PhotoMetadata::doSaveImage, image);
    m_saveWatcher.setFuture(future);
    Q_EMIT savingChanged();
}

void PhotoMetadata::save()
{
    QFuture<void> future = QtConcurrent::run(this, &PhotoMetadata::doSave);
    m_saveWatcher.setFuture(future);
    Q_EMIT savingChanged();
}

void PhotoMetadata::doSaveImage(QImage image)
{
    image.save(m_fileName);
    updateThumbnail(image);
    m_image->writeMetadata();
}


void PhotoMetadata::doSave()
{
    m_image->writeMetadata();
}
}
