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

#ifndef PHOTO_IMAGE_PROVIDER_H_
#define PHOTO_IMAGE_PROVIDER_H_

#include <QtQuick/QQuickImageProvider>
#include <QtGui/QImage>
#include <QtCore/QString>
#include <QtCore/QSize>

class PhotoImageProvider : public QQuickImageProvider
{
public:
    static const char* PROVIDER_ID;

    PhotoImageProvider();
    virtual ~PhotoImageProvider();

    virtual QImage requestImage(const QString& id, QSize* size,
                                const QSize& requestedSize);
};

#endif // PHOTO_IMAGE_PROVIDER_H_
