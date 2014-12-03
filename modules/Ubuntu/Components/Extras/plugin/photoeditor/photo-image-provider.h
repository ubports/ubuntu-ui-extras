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
 * Lucas Beeler <lucas@yorba.org>
 * Jim Nelson <jim@yorba.org>
 * Ugo Riboni <ugo.riboni@canonical.com>
*/

#ifndef PHOTO_IMAGE_PROVIDER_H_
#define PHOTO_IMAGE_PROVIDER_H_

// util
#include "orientation.h"

#include <QDateTime>
#include <QFileInfo>
#include <QImage>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QQuickImageProvider>
#include <QSize>
#include <QString>
#include <QUrl>

/*!
 * We use a custom image provider for the following reasons:
 *
 * 1. QML's image loader does not respect EXIF orientation.  This provider will
 *    rotate and mirror the image as necessary.
 *
 * 2. QML's image caching appears to be directly related to image size (scaling)
 *    which leads to thrashing when animating a thumbnail or loading images at
 *    various sizes.
 *    The strategy here is to cache the largest requested size of the image
 *    and downscale it if smaller versions are requested. This minimizes
 *    expensive JPEG load-and-decodes.
 *
 * 3. Logging allows for monitoring of all image I/O, useful when debugging and
 *    optimizing, and signals can be emitted to monitor and test cache operation.
 *
 * 4. The QML cache does not check if a file has been modified on disk after it
 *    was cached, but our cache does. You should always set Image.cache to false
 *    when loading images from this provider in QML.
 */
class PhotoImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT

public:
    static const char* PROVIDER_ID;
    static const char* PROVIDER_ID_SCHEME;

    PhotoImageProvider();
    virtual ~PhotoImageProvider();

    virtual QImage requestImage(const QString& id, QSize* size,
                                const QSize& requestedSize);

    void setLogging(bool enableLogging);
    void setEmitCacheSignals(bool emitCacheSignals);

Q_SIGNALS:
    void cacheHit(QString id, QSize size);
    void cacheMiss(QString id, QSize size, bool wasStale);
    void cacheAdd(QString id, QSize size, QSize cachedSize);

private:
    class CachedImage {
    public:
        const QString id;
        QMutex imageMutex;

        // these fields should only be accessed when imageMutex_ is locked
        QImage image;
        QSize fullSize;
        Orientation orientation;
        QDateTime cachedAt;

        // the following should only be accessed when cacheMutex_ is locked; the
        // counter controls removing a CachedImage entry from the cache table
        int cleanCount;
        uint byteCount;

        CachedImage(const QString& id);

        void storeImage(const QImage& newImage, const QSize& newFullSize,
                        Orientation newOrientation);
        bool isFullSized() const;
        bool isReady() const;
        bool isCacheHit(const QSize& requestedSize) const;
    };

    QMap<QString, CachedImage*> m_cache;
    QList<QString> m_cachingOrder;
    QMutex m_cacheMutex;
    long m_cachedBytes;
    bool m_logImageLoading;
    bool m_emitCacheSignals;

    static QSize orientSize(const QSize& size, Orientation orientation);

    CachedImage* claimCachedImageEntry(const QString& id, QString& loggingStr);

    QImage fetchCachedImage(CachedImage* cachedImage, const QSize& requestedSize,
                              uint* bytesLoaded, QString& loggingStr);

    void releaseCachedImageEntry(CachedImage* cachedImage, uint bytesLoaded);
};

#endif // PHOTO_IMAGE_PROVIDER_H_
