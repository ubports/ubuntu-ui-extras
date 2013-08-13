/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "imageresizer.h"
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>
#include <QDir>

ImageResizer::ImageResizer(QObject *parent) :
    QObject(parent),
    m_resizedImage(NULL),
    m_largestSide(2048),
    m_quality(85)
{
}

QString ImageResizer::file() const
{
    return m_file;
}

QString ImageResizer::resizedFile() const
{
    return (m_resizedImage) ? m_resizedImage->fileName() : "";
}

int ImageResizer::largestSide() const
{
    return m_largestSide;
}

int ImageResizer::quality() const
{
    return m_quality;
}

void ImageResizer::setFile(const QString& file)
{
    if (m_file != file) {
        m_file = file;
        Q_EMIT fileChanged();
        resizeImage();
    }
}

void ImageResizer::setLargestSide(int largestSide)
{
    if (largestSide <= 0) {
        largestSide = 1;
    }

    if (m_largestSide != largestSide) {
        m_largestSide = largestSide;
        Q_EMIT largestSideChanged();
        resizeImage();
    }
}

void ImageResizer::setQuality(int quality)
{
    if (quality < 0) {
        quality = 0;
    } else if (quality > 100) {
        quality = 100;
    }
    if (m_quality != quality) {
        m_quality = quality;
        Q_EMIT qualityChanged();
        resizeImage();
    }
}

void ImageResizer::resizeImage()
{
    QImage image;
    QFileInfo source(m_file);
    if (m_file.isEmpty() || m_largestSide <= 0 ||
        !source.exists() || !image.load(m_file)) {
        if (m_resizedImage) {
            delete m_resizedImage;
            m_resizedImage = NULL;
        }
        Q_EMIT resizedFileChanged();
        return;
    }

    QImage scaledImage;
    if (image.height() > image.width()) {
        if (m_largestSide > image.height()) {

            scaledImage = image;
        } else {
            scaledImage = image.scaledToHeight(m_largestSide);
        }
    } else {
        if (m_largestSide >= image.width()) {
            scaledImage = image;
        } else {
            scaledImage = image.scaledToWidth(m_largestSide);
        }
    }

    QString resizedFilePath = QDir::tempPath() + QDir::separator() +
                              "share-app.resized.XXXXXX." + source.suffix();

    if (m_resizedImage) {
        delete m_resizedImage;
        m_resizedImage = NULL;
    }
    m_resizedImage = new QTemporaryFile(resizedFilePath, this);
    m_resizedImage->open();
    QImageWriter writer(m_resizedImage, source.suffix().toLatin1());
    writer.write(scaledImage);
    writer.setQuality(m_quality);
    m_resizedImage->close();

    Q_EMIT resizedFileChanged();
}
