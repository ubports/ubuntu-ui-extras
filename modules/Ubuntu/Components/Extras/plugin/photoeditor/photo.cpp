/*
 * Copyright (C) 2011-2014 Canonical Ltd
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
 * Jim Nelson <jim@yorba.org>
 * Lucas Beeler <lucas@yorba.org>
 * Charles Lindsay <chaz@yorba.org>
 * Eric Gregory <eric@yorba.org>
 * Clint Rogers <clinton@yorba.org>
 * Ugo Riboni <ugo.riboni@canonical.com>
 */

#include "photo.h"
#include "photo-edit-command.h"
#include "photo-edit-thread.h"

// medialoader
#include "photo-metadata.h"

// util
#include "imaging.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QStack>
#include <QStandardPaths>

/*!
 * \brief Photo::isValid
 * \param file
 * \return
 */
bool Photo::isValid(const QFileInfo& file)
{
    QImageReader reader(file.filePath());
    QByteArray format = reader.format();

    if (QString(format).toLower() == "tiff") {
        // QImageReader.canRead() will detect some raw files as readable TIFFs,
        // though QImage will fail to load them.
        QString extension = file.suffix().toLower();
        if (extension != "tiff" && extension != "tif")
            return false;
    }

    PhotoMetadata* tmp = PhotoMetadata::fromFile(file);
    if (tmp == NULL)
        return false;

    delete tmp;
    return reader.canRead() &&
            QImageWriter::supportedImageFormats().contains(reader.format());
}

/*!
 * \brief Photo::Photo
 * \param file
 */
Photo::Photo()
    : QObject(),
    m_editThread(0),
    m_originalSize(),
    m_orientation(TOP_LEFT_ORIGIN),
    m_originalOrientation(TOP_LEFT_ORIGIN),
    m_busy(false)
{
}

void Photo::setPath(QString path)
{
    if (QFileInfo(path).absoluteFilePath() != m_file.absoluteFilePath()) {
        QFileInfo newFile(path);
        if (newFile.exists() && newFile.isFile()) {
            QByteArray format = QImageReader(newFile.absoluteFilePath()).format();
            m_fileFormat = QString(format).toLower();
            if (m_fileFormat == "jpg") // Why does Qt expose two different names here?
                m_fileFormat = "jpeg";

            m_file = newFile;
            Q_EMIT pathChanged();

            if (fileFormatHasMetadata()) {
                PhotoMetadata* metadata = PhotoMetadata::fromFile(newFile.absoluteFilePath());
                m_originalOrientation = metadata->orientation();
                m_orientation = metadata->orientation();
                delete metadata;
                Q_EMIT orientationChanged();
            }
        }
    }
}

QString Photo::path() const
{
    return m_file.absoluteFilePath();
}

QFileInfo Photo::file() const
{
    return m_file;
}

/*!
 * \brief Photo::~Photo
 */
Photo::~Photo()
{
    if (m_editThread) {
        m_editThread->wait();
        finishEditing();
    }
}

/*!
 * \brief Photo::image Returns the photo in full size.
 * \param respectOrientation if set to true, the photo is rotated according to the EXIF information
 * \return The image in full size
 */
QImage Photo::image(bool respectOrientation, const QSize &scaleSize)
{
    QImageReader imageReader(m_file.filePath(), m_fileFormat.toStdString().c_str());
    QSize imageSize = imageReader.size();
    if (scaleSize.isValid()) {
        QSize size = imageSize;
        size.scale(scaleSize, Qt::KeepAspectRatioByExpanding);
        imageReader.setScaledSize(size);
    }
    QImage image = imageReader.read();
    if (!image.isNull() && respectOrientation && fileFormatHasOrientation()) {
        image = image.transformed(
                    OrientationCorrection::fromOrientation(orientation())
                    .toTransform());
    }

    return image;
}

/*!
 * \brief Photo::orientation
 * \return
 */
Orientation Photo::orientation() const
{  
    return m_orientation;
}

Orientation Photo::originalOrientation() const
{
    return m_originalOrientation;
}

void Photo::refreshFromDisk()
{
    if (fileFormatHasMetadata()) {
        PhotoMetadata* metadata = PhotoMetadata::fromFile(m_file.absoluteFilePath());
        qDebug() << "Refreshing orient." << m_orientation << "to" << metadata->orientation();
        m_originalOrientation = metadata->orientation();
        m_orientation = metadata->orientation();
        delete metadata;
        Q_EMIT orientationChanged();
    }

    Q_EMIT dataChanged();
}

/*!
 * \brief Photo::rotateRight
 */
void Photo::rotateRight()
{
    Orientation current = fileFormatHasOrientation() ? orientation() :
                                                       TOP_LEFT_ORIGIN;
    Orientation rotated = OrientationCorrection::rotateOrientation(current,
                                                                   false);
    qDebug() << " Rotate from orientation " << current << "to" << rotated;

    PhotoEditCommand command;
    command.type = EDIT_ROTATE;
    command.orientation = rotated;
    asyncEdit(command);
}

/*!
 * \brief Photo::autoEnhance
 */
void Photo::autoEnhance()
{
    PhotoEditCommand command;
    command.type = EDIT_ENHANCE;
    asyncEdit(command);
}

/*!
 * \brief Photo::exposureCompensation Changes the brightnes of the image
 * \param value Value for the compensation. -1.0 moves the image into total black.
 * +1.0 to total white. 0.0 leaves it as it is.
 */
void Photo::exposureCompensation(qreal value)
{
    PhotoEditCommand command;
    command.type = EDIT_COMPENSATE_EXPOSURE;
    command.exposureCompensation = value;
    asyncEdit(command);
}

/*!
 * \brief Photo::colorBalance adjusts the colors
 * \param brightness 0 is total dark, 1 is as the original, grater than 1 is brigther
 * \param contrast from 0 maybe 5. 1 is as the original
 * \param saturation from 0 maybe 5. 1 is as the original
 * \param hue from 0 to 360. 0 and 360 is as the original
 */
void Photo::colorBalance(qreal brightness, qreal contrast, qreal saturation, qreal hue)
{
    PhotoEditCommand next_state;
    next_state.colorBalance_ = QVector4D(brightness, contrast, saturation, hue);
    asyncEdit(next_state);
}

/*!
 * \brief Photo::crop
 * Specify all coords in [0,1].
 * \param vrect
 */
void Photo::crop(QVariant vrect)
{
    QRectF ratio_crop_rect = vrect.toRectF();

    QSize image_size = originalSize(orientation());

    // Integer truncation is good enough here.
    int x = ratio_crop_rect.x() * image_size.width();
    int y = ratio_crop_rect.y() * image_size.height();
    int width = ratio_crop_rect.width() * image_size.width();
    int height = ratio_crop_rect.height() * image_size.height();

    if (x < 0 || y < 0 || width <= 0 || height <= 0 ||
        x + width > image_size.width() || y + height > image_size.height()) {
        qWarning() << "Invalid cropping rectangle";
        return;
    }

    PhotoEditCommand command;
    command.type = EDIT_CROP;
    command.crop_rectangle = QRect(x, y, width, height);
    asyncEdit(command);
}

/*!
 * \brief Photo::originalSize Returns the original image size translated to the desired orientation
 * \param orientation
 * \return Returns the original image size translated to the desired orientation
 */
QSize Photo::originalSize(Orientation orientation)
{
    if (!m_originalSize.isValid()) {
        QImage original(m_file.filePath(),
                        m_fileFormat.toStdString().c_str());
        if (fileFormatHasOrientation()) {
            original =
                    original.transformed(OrientationCorrection::fromOrientation(
                                             m_originalOrientation).toTransform());
        }

        m_originalSize = original.size();
    }

    QSize rotated_size = m_originalSize;

    if (orientation != ORIGINAL_ORIENTATION) {
        OrientationCorrection original_correction =
                OrientationCorrection::fromOrientation(m_originalOrientation);
        OrientationCorrection out_correction =
                OrientationCorrection::fromOrientation(orientation);
        int degrees_rotation =
                original_correction.getNormalizedRotationDifference(out_correction);

        if (degrees_rotation == 90 || degrees_rotation == 270)
            rotated_size.transpose();
    }

    return rotated_size;
}

/*!
 * \brief Photo::asyncEdit does edit the photo according to the given command
 * in a background thread.
 * \param The command defining the edit operation to perform.
 */
void Photo::asyncEdit(const PhotoEditCommand& command)
{
    if (m_busy) {
        qWarning() << "Can't start edit operation while another one is running.";
        return;
    }
    m_busy = true;
    Q_EMIT busyChanged();
    m_editThread = new PhotoEditThread(this, command);
    connect(m_editThread, SIGNAL(finished()), this, SLOT(finishEditing()));
    m_editThread->start();
}

/*!
 * \brief Photo::finishEditing do all the updates once the editing is done
 */
void Photo::finishEditing()
{
    if (!m_editThread || m_editThread->isRunning())
        return;

    const PhotoEditCommand &state = m_editThread->command();

    m_editThread->deleteLater();
    m_editThread = 0;
    m_busy = false;

    refreshFromDisk();

    Q_EMIT busyChanged();
    Q_EMIT editFinished();
}

/*!
 * \brief Photo::fileFormat returns the file format as QString
 * \return
 */
const QString &Photo::fileFormat() const
{
    return m_fileFormat;
}

/*!
 * \brief Photo::fileFormatHasMetadata
 * \return
 */
bool Photo::fileFormatHasMetadata() const
{
    return (m_fileFormat == "jpeg" || m_fileFormat == "tiff" ||
            m_fileFormat == "png");
}

/*!
 * \brief Photo::fileFormatHasOrientation
 * \return
 */
bool Photo::fileFormatHasOrientation() const
{
    return (m_fileFormat == "jpeg");
}

/*!
 * \brief Photo::originalSize
 * \return
 */
const QSize &Photo::originalSize()
{
    originalSize(ORIGINAL_ORIENTATION);
    return m_originalSize;
}

/*!
 * \brief Photo::busy return true if there is an editing operation in progress
 * \return
 */
bool Photo::busy() const
{
    return m_busy;
}
