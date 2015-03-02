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

#include "photo-data.h"
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
bool PhotoData::isValid(const QFileInfo& file)
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
PhotoData::PhotoData()
    : QObject(),
    m_editThread(0),
    m_busy(false),
    m_orientation(TOP_LEFT_ORIGIN)
{
}

void PhotoData::setPath(QString path)
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
                m_orientation = metadata->orientation();
                delete metadata;
                Q_EMIT orientationChanged();
            }
        }
    }
}

QString PhotoData::path() const
{
    return m_file.absoluteFilePath();
}

QFileInfo PhotoData::file() const
{
    return m_file;
}

/*!
 * \brief Photo::~Photo
 */
PhotoData::~PhotoData()
{
    if (m_editThread) {
        m_editThread->wait();
        finishEditing();
    }
}

/*!
 * \brief Photo::orientation
 * \return
 */
Orientation PhotoData::orientation() const
{  
    return m_orientation;
}

void PhotoData::refreshFromDisk()
{
    if (fileFormatHasMetadata()) {
        PhotoMetadata* metadata = PhotoMetadata::fromFile(m_file.absoluteFilePath());
        qDebug() << "Refreshing orient." << m_orientation << "to" << metadata->orientation();
        m_orientation = metadata->orientation();
        delete metadata;
        Q_EMIT orientationChanged();
    }

    Q_EMIT dataChanged();
}

/*!
 * \brief Photo::rotateRight
 */
void PhotoData::rotateRight()
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
void PhotoData::autoEnhance()
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
void PhotoData::exposureCompensation(qreal value)
{
    PhotoEditCommand command;
    command.type = EDIT_COMPENSATE_EXPOSURE;
    command.exposureCompensation = value;
    asyncEdit(command);
}

/*!
 * \brief Photo::crop
 * Specify all coords in [0.0, 1.0], where 1.0 is the full size of the image.
 * They will be clamped to this range if you don't.
 * \param vrect the rectangle specifying the region to be cropped
 */
void PhotoData::crop(QVariant vrect)
{
    PhotoEditCommand command;
    command.type = EDIT_CROP;
    command.crop_rectangle = vrect.toRectF();
    asyncEdit(command);
}

/*!
 * \brief Photo::asyncEdit does edit the photo according to the given command
 * in a background thread.
 * \param The command defining the edit operation to perform.
 */
void PhotoData::asyncEdit(const PhotoEditCommand& command)
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
void PhotoData::finishEditing()
{
    if (!m_editThread || m_editThread->isRunning())
        return;

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
const QString &PhotoData::fileFormat() const
{
    return m_fileFormat;
}

/*!
 * \brief Photo::fileFormatHasMetadata
 * \return
 */
bool PhotoData::fileFormatHasMetadata() const
{
    return (m_fileFormat == "jpeg" || m_fileFormat == "tiff" ||
            m_fileFormat == "png");
}

/*!
 * \brief Photo::fileFormatHasOrientation
 * \return
 */
bool PhotoData::fileFormatHasOrientation() const
{
    return (m_fileFormat == "jpeg");
}

/*!
 * \brief Photo::busy return true if there is an editing operation in progress
 * \return
 */
bool PhotoData::busy() const
{
    return m_busy;
}
