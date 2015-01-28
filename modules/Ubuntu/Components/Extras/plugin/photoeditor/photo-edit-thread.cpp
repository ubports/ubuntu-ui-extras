/*
 * Copyright (C) 2013-2014 Canonical Ltd
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

#include "photo-edit-thread.h"
#include "photo-data.h"

// medialoader
#include "photo-metadata.h"

// util
#include "imaging.h"

#include <QDebug>

/*!
 * \brief PhotoEditThread::PhotoEditThread
 */
PhotoEditThread::PhotoEditThread(PhotoData *photo, const PhotoEditCommand &command)
    : QThread(),
      m_photo(photo),
      m_command(command)
{
}

/*!
 * \brief PhotoEditThread::command resturns the editing command used for this processing
 * \return
 */
const PhotoEditCommand &PhotoEditThread::command() const
{
    return m_command;
}

/*!
 * \brief PhotoEditThread::run \reimp
 */
void PhotoEditThread::run()
{
    // The only operation in which we don't have to work on the actual image
    // pixels is image rotation in the case where we can simply change the
    // metadata rotation field.
    if (m_command.type == EDIT_ROTATE && m_photo->fileFormatHasOrientation()) {
        handleSimpleMetadataRotation(m_command);
        return;
    }

    // In all other cases we load the image, do the work, and save it back.
    QImage image(m_photo->file().filePath(), m_photo->fileFormat().toStdString().c_str());
    if (image.isNull()) {
        qWarning() << "Error loading" << m_photo->file().filePath() << "for editing";
        return;
    }

    // Copy all metadata from the original image so that we can save it to the
    // new one after modifying the pixels.
    PhotoMetadata* original = PhotoMetadata::fromFile(m_photo->file());

    // If the photo was previously rotated through metadata and we are editing
    // the actual pixels, first rotate the image to match the orientation so
    // that the correct pixels are edited.
    // Obviously don't do this in the case we have been asked to do a rotation
    // operation on the pixels, as we would do it later as the operation itself.
    if (m_photo->fileFormatHasOrientation() && m_command.type != EDIT_ROTATE) {
        Orientation orientation = m_photo->orientation();
        QTransform transform = OrientationCorrection::fromOrientation(orientation).toTransform();
        image = image.transformed(transform);
    }

    if (m_command.type == EDIT_ROTATE) {
        QTransform transform = OrientationCorrection::fromOrientation(m_command.orientation).toTransform();
        image = image.transformed(transform);
    } else if (m_command.type == EDIT_CROP) {
        QRect rect;
        rect.setX(qBound(0.0, m_command.crop_rectangle.x(), 1.0) * image.width());
        rect.setY(qBound(0.0, m_command.crop_rectangle.y(), 1.0) * image.height());
        rect.setWidth(qBound(0.0, m_command.crop_rectangle.width(), 1.0) * image.width());
        rect.setHeight(qBound(0.0, m_command.crop_rectangle.height(), 1.0) * image.height());

        image = image.copy(rect);
    } else if (m_command.type == EDIT_ENHANCE) {
        image = enhanceImage(image);
    } else if (m_command.type == EDIT_COMPENSATE_EXPOSURE) {
        image = compensateExposure(image, m_command.exposureCompensation);
    } else {
        qWarning() << "Edit thread running with unknown or no operation.";
        return;
    }

    bool saved = image.save(m_photo->file().filePath(),
                            m_photo->fileFormat().toStdString().c_str(), 90);
    if (!saved)
        qWarning() << "Error saving edited" << m_photo->file().filePath();

    PhotoMetadata* copy = PhotoMetadata::fromFile(m_photo->file());
    original->copyTo(copy);
    copy->setOrientation(TOP_LEFT_ORIGIN); // reset previous orientation
    copy->updateThumbnail(image);
    copy->save();

    delete original;
    delete copy;
}

/*!
 * \brief PhotoEditThread::handleSimpleMetadataRotation
 * Handler for the case of an image whose only change is to its
 * orientation; used to skip re-encoding of JPEGs.
 * \param state
 */
void PhotoEditThread::handleSimpleMetadataRotation(const PhotoEditCommand& state)
{
    PhotoMetadata* metadata = PhotoMetadata::fromFile(m_photo->file());
    metadata->setOrientation(state.orientation);
    metadata->save();
    delete(metadata);
}

/*!
 * \brief PhotoEditThread::enhanceImage
 */
QImage PhotoEditThread::enhanceImage(const QImage& image)
{
    int width = image.width();
    int height = image.height();

    QImage sample_img = (image.width() > 400) ? image.scaledToWidth(400) : image;

    AutoEnhanceTransformation enhance = AutoEnhanceTransformation(sample_img);

    QImage::Format dest_format = image.format();

    // Can't write into indexed images, due to a limitation in Qt.
    if (dest_format == QImage::Format_Indexed8)
        dest_format = QImage::Format_RGB32;

    QImage enhanced_image(width, height, dest_format);

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            QColor px = enhance.transformPixel(
                        QColor(image.pixel(i, j)));
            enhanced_image.setPixel(i, j, px.rgb());
        }
    }

    return enhanced_image;
}

/*!
 * \brief PhotoEditThread::compensateExposure Compensates the exposure
 * Compensating the exposure is a change in brightnes
 * \param image Image to change the brightnes
 * \param compansation -1.0 is total dark, +1.0 is total bright
 * \return The image with adjusted brightnes
 */
QImage PhotoEditThread::compensateExposure(const QImage &image, qreal compensation)
{
    int shift = qBound(-255, (int)(255*compensation), 255);
    QImage result(image.width(), image.height(), image.format());

    for (int j = 0; j < image.height(); j++) {
        for (int i = 0; i <image.width(); i++) {
            QColor px = image.pixel(i, j);
            int red = qBound(0, px.red() + shift, 255);
            int green = qBound(0, px.green() + shift, 255);
            int blue = qBound(0, px.blue() + shift, 255);
            result.setPixel(i, j, qRgb(red, green, blue));
        }
    }

    return result;
}
