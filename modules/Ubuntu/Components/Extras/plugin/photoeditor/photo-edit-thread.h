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

#ifndef GALLERY_PHOTO_EDIT_THREAD_H_
#define GALLERY_PHOTO_EDIT_THREAD_H_

#include "photo-caches.h"
#include "photo-edit-command.h"

// util
#include "orientation.h"

#include <QImage>
#include <QThread>
#include <QUrl>

class PhotoData;

/*!
 * \brief The PhotoEditThread class
 */
class PhotoEditThread: public QThread
{
    Q_OBJECT
public:
    PhotoEditThread(PhotoData *photo, const PhotoEditCommand& command);

    const PhotoEditCommand& command() const;

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QImage enhanceImage(const QImage& image);
    QImage compensateExposure(const QImage& image, qreal compansation);
    QImage doColorBalance(const QImage& image, qreal brightness, qreal contrast, qreal saturation, qreal hue);
    void handleSimpleMetadataRotation(const PhotoEditCommand& state);

    PhotoData *m_photo;
    PhotoEditCommand m_command;
};

#endif
