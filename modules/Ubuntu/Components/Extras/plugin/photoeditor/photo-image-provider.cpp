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

#include "photo-image-provider.h"
#include "photo-metadata.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QImageReader>
#include <QSize>
#include <QUrlQuery>

const char* PhotoImageProvider::PROVIDER_ID = "photo";
const char* PhotoImageProvider::PROVIDER_ID_SCHEME = "image://photo/";

const long MAX_CACHE_BYTES = 20L * 1024L * 1024L;

/*!
 * \brief PhotoImageProvider::PhotoImageProvider
 */
PhotoImageProvider::PhotoImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image),
      m_cachedBytes(0),
      m_logImageLoading(false),
      m_emitCacheSignals(false)
{
}

/*!
 * \brief PhotoImageProvider::~PhotoImageProvider
 */
PhotoImageProvider::~PhotoImageProvider()
{
    // NOTE: This assumes that we are not receiving requests any longer
    while (!m_cachingOrder.isEmpty())
        delete m_cache.value(m_cachingOrder.takeFirst());
}

#define LOG_IMAGE_STATUS(status) { \
    if (m_logImageLoading) \
    loggingStr += status; \
    }

/*!
 * \brief PhotoImageProvider::requestImage
 * \param id
 * \param size
 * \param requestedSize
 * \return
 */
QImage PhotoImageProvider::requestImage(const QString& id,
                                                  QSize* size, const QSize& requestedSize)
{
    // for LOG_IMAGE_STATUS
    QString loggingStr = "";
    QElapsedTimer timer;
    if (m_logImageLoading) timer.start();

    QUrl url(id);

    QImage readyImage;
    uint bytesLoaded = 0;

    CachedImage* cachedImage = claimCachedImageEntry(id, loggingStr);
    Q_ASSERT(cachedImage != NULL);

    readyImage = fetchCachedImage(cachedImage, requestedSize, &bytesLoaded,
                                  loggingStr);
    if (readyImage.isNull())
        LOG_IMAGE_STATUS("load-failure ");

    releaseCachedImageEntry(cachedImage, bytesLoaded);

    if (m_logImageLoading) {
        if (bytesLoaded > 0) {
            qDebug("%s %s req:%dx%d ret:%dx%d cache:%ldb/%d loaded:%db time:%lldms", qPrintable(loggingStr),
                   qPrintable(id), requestedSize.width(), requestedSize.height(), readyImage.width(),
                   readyImage.height(), m_cachedBytes, m_cache.size(), bytesLoaded,
                   timer.elapsed());
        } else {
            qDebug("%s %s req:%dx%d ret:%dx%d cache:%ldb/%d time:%lldms", qPrintable(loggingStr),
                   qPrintable(id), requestedSize.width(), requestedSize.height(), readyImage.width(),
                   readyImage.height(), m_cachedBytes, m_cache.size(), timer.elapsed());
        }
    }

    if (size != NULL)
        *size = readyImage.size();

    return readyImage;
}

/*!
 * \brief PhotoImageProvider::setLogging enables to print photo loading
 * times to stout
 * \param enableLogging
 */
void PhotoImageProvider::setLogging(bool enableLogging)
{
    m_logImageLoading = enableLogging;
}

/*!
 * \brief PhotoImageProvider::setEmitCacheSignals enabled emitting signals to
 * track the status of the internal cache.
 * \param emitCacheSignals
 */
void PhotoImageProvider::setEmitCacheSignals(bool emitCacheSignals)
{
    m_emitCacheSignals = emitCacheSignals;
}

/*!
 * \brief PhotoImageProvider::claim_cached_image_entry
 * Returns a CachedImage with an inUseCount > 0, meaning it cannot be
 * removed from the cache until released
 * \param id
 * \param loggingStr
 * \return
 */
PhotoImageProvider::CachedImage* PhotoImageProvider::claimCachedImageEntry(
        const QString& id, QString& loggingStr)
{
    // lock the cache table and retrieve the element for the cached image; if
    // not found, create one as a placeholder
    m_cacheMutex.lock();

    CachedImage* cachedImage = m_cache.value(id, NULL);
    if (cachedImage != NULL) {
        // remove CachedImage before prepending to FIFO
        m_cachingOrder.removeOne(id);
    } else {
        cachedImage = new CachedImage(id);
        m_cache.insert(id, cachedImage);
        LOG_IMAGE_STATUS("new-cache-entry ");
    }

    // add to front of FIFO
    m_cachingOrder.prepend(id);

    // should be the same size, always
    Q_ASSERT(m_cache.size() == m_cachingOrder.size());

    // claim the CachedImage *while cacheMutex_ is locked* ... this prevents the
    // CachedImage from being removed from the cache while its being filled
    cachedImage->cleanCount++;

    m_cacheMutex.unlock();

    return cachedImage;
}

/*!
 * \brief PhotoImageProvider::fetch_cached_image Inspects and loads a proper image
 * Inspects and loads a proper image for this request into the CachedImage
 * \param cachedImage
 * \param requestedSize
 * \param bytesLoaded
 * \param loggingStr
 * \return
 */
QImage PhotoImageProvider::fetchCachedImage(CachedImage *cachedImage,
                                            const QSize& requestedSize,
                                            uint* bytesLoaded,
                                            QString& loggingStr)
{
    Q_ASSERT(cachedImage != NULL);

    QString file = QUrl(cachedImage->id).path();

    // the final image returned to the user
    QImage readyImage;
    Q_ASSERT(readyImage.isNull());

    // lock the cached image itself to access
    cachedImage->imageMutex.lock();

    // Depending on the file system used the last modified date of a file
    // might have a really low resolution (2s for FAT32, 1s for ext3). Therefore
    // we have to take the worse case and accept that there will be additional cache
    // misses when an image is requested again just after it has been cached.
    // There is no alternative to this other than accepting false cache hits, which
    // would result in bugs in the application.
    QFileInfo photoFile(file);
    QDateTime fileLastModified = photoFile.lastModified();
    fileLastModified = fileLastModified.addSecs(2);

    // if image is available, see if a fit
    if (cachedImage->isCacheHit(requestedSize)) {
        if (cachedImage->cachedAt > fileLastModified) {
            readyImage = cachedImage->image;
            LOG_IMAGE_STATUS("cache-hit ");
            if (m_emitCacheSignals) Q_EMIT cacheHit(cachedImage->id, requestedSize);
        } else {
            // if the file was modified after the image was cached, reload it
            LOG_IMAGE_STATUS("cache-stale ");
            if (m_emitCacheSignals) cacheMiss(cachedImage->id, requestedSize, true);
        }
    } else {
        LOG_IMAGE_STATUS("cache-miss ");
        if (m_emitCacheSignals) cacheMiss(cachedImage->id, requestedSize, false);
    }

    if (bytesLoaded != NULL)
        *bytesLoaded = 0;

    // if unavailable or stale, load now
    if (readyImage.isNull()) {
        QImageReader reader(file);

        // load file's original size
        QSize fullSize = reader.size();
        QSize loadSize(fullSize);

        // use scaled load-and-decode if size has been requested
        if (fullSize.isValid() && (requestedSize.width() > 0 || requestedSize.height() > 0)) {
            loadSize.scale(requestedSize, Qt::KeepAspectRatio);
            if (loadSize.width() > fullSize.width() || loadSize.height() > fullSize.height())
                loadSize = fullSize;
        }

        if (loadSize != fullSize) {
            LOG_IMAGE_STATUS("scaled-load ");

            // configure reader for scaled load-and-decode
            reader.setScaledSize(loadSize);
        } else {
            LOG_IMAGE_STATUS("full-load ");
        }

        readyImage = reader.read();
        if (!readyImage.isNull()) {
            if (!fullSize.isValid())
                fullSize = readyImage.size();

            Orientation orientation = TOP_LEFT_ORIGIN;
            std::auto_ptr<PhotoMetadata> metadata(PhotoMetadata::fromFile(file));
            if (metadata.get() != NULL)
                orientation = metadata->orientation();

            // rotate image if not TOP LEFT
            if (orientation != TOP_LEFT_ORIGIN)  {
                readyImage = readyImage.transformed(
                            OrientationCorrection::fromOrientation(orientation).toTransform());
            }

            // If we are reloading an image, the cache total byte count will be the
            // difference in size between the old image and the new one (could be negative, for
            // example in cases of cropping).
            // If we are not reloading, then the current count will be zero and the total byte
            // count will be the full size of the newly loaded image.
            int currentByteCount = readyImage.byteCount();

            cachedImage->storeImage(readyImage, fullSize, orientation);
            if (m_emitCacheSignals) Q_EMIT cacheAdd(cachedImage->id, requestedSize, loadSize);

            if (bytesLoaded != NULL)
                *bytesLoaded = readyImage.byteCount() - currentByteCount;
        } else {
            qDebug("Unable to load %s: %s", qPrintable(cachedImage->id),
                   qPrintable(reader.errorString()));
        }
    } else {
        // if the image comes from the cache and the requested size is smaller
        // than what we cached, scale the image before returning it
        if (requestedSize.isValid()) {
            readyImage = readyImage.scaled(requestedSize, Qt::KeepAspectRatio);
        }
    }

    cachedImage->imageMutex.unlock();

    return readyImage;
}

/*!
 * \brief PhotoImageProvider::release_cached_image_entry
 * Releases a CachedImage to the cache; takes its bytes loaded (0 if nothing
 * was loaded) and returns the current cached byte total
 * \param cachedImage
 * \param bytesLoaded
 * \param currentCacheEntries
 */
void PhotoImageProvider::releaseCachedImageEntry
(CachedImage *cachedImage, uint bytesLoaded)
{
    Q_ASSERT(cachedImage != NULL);

    // update total cached bytes and remove excess bytes
    m_cacheMutex.lock();

    m_cachedBytes += bytesLoaded;

    // update the CachedImage use count and byte count inside of *cachedMutex_ lock*
    Q_ASSERT(cachedImage->cleanCount > 0);
    cachedImage->cleanCount--;
    if (bytesLoaded != 0)
        cachedImage->byteCount = bytesLoaded;

    // trim the cache
    QList<CachedImage*> dropList;
    while (m_cachedBytes > MAX_CACHE_BYTES && !m_cachingOrder.isEmpty()) {
        QString droppedFile = m_cachingOrder.takeLast();

        CachedImage* droppedCachedImage = m_cache.value(droppedFile);
        Q_ASSERT(droppedCachedImage != NULL);

        // for simplicity, stop when dropped item is in use or doesn't contain
        // an image (which it won't for too long) ... will clean up next time
        // through
        if (droppedCachedImage->cleanCount > 0) {
            m_cachingOrder.append(droppedFile);

            break;
        }

        // remove from map
        m_cache.remove(droppedFile);

        // decrement total cached size
        m_cachedBytes -= droppedCachedImage->byteCount;
        Q_ASSERT(m_cachedBytes >= 0);

        dropList.append(droppedCachedImage);
    }

    // coherency is good
    Q_ASSERT(m_cache.size() == m_cachingOrder.size());

    m_cacheMutex.unlock();

    // perform actual deletion outside of lock
    while (!dropList.isEmpty())
        delete dropList.takeFirst();
}

/*!
 * \brief PhotoImageProvider::orientSize
 * \param size
 * \param orientation
 * \return
 */
QSize PhotoImageProvider::orientSize(const QSize& size, Orientation orientation)
{
    switch (orientation) {
    case LEFT_TOP_ORIGIN:
    case RIGHT_TOP_ORIGIN:
    case RIGHT_BOTTOM_ORIGIN:
    case LEFT_BOTTOM_ORIGIN:
        return QSize(size.height(), size.width());
        break;

    default:
        // no change
        return size;
    }
}

/*!
 * \brief PhotoImageProvider::CachedImage::CachedImage
 * \param id the full URI of the image
 * \param fileName the filename for the URI (the file itself)
 */
PhotoImageProvider::CachedImage::CachedImage(const QString& id)
    : id(id), orientation(TOP_LEFT_ORIGIN), cleanCount(0), byteCount(0)
{
}

/*!
 * \brief PhotoImageProvider::CachedImage::storeImage
 * Importand: the following should only be called when imageMutex_ is locked
 * \param image
 * \param fullSize
 * \param orientation
 */
void PhotoImageProvider::CachedImage::storeImage(const QImage& newImage,
                                                           const QSize& newFullSize,
                                                           Orientation newOrientation)
{
    image = newImage;
    fullSize = orientSize(newFullSize, orientation);
    orientation = newOrientation;
    cachedAt = QDateTime::currentDateTime();
}

/*!
 * \brief PhotoImageProvider::CachedImage::isReady
 * \return
 */
bool PhotoImageProvider::CachedImage::isReady() const
{
    return !image.isNull();
}

/*!
 * \brief PhotoImageProvider::CachedImage::isFullSized
 * \return
 */
bool PhotoImageProvider::CachedImage::isFullSized() const
{
    return isReady() && (image.size() == fullSize);
}

/*!
 * \brief PhotoImageProvider::CachedImage::isCacheHit
 * \param requestedSize
 * \return
 */
bool PhotoImageProvider::CachedImage::isCacheHit(const QSize& requestedSize) const
{
    if (!isReady())
        return false;

    if (isFullSized())
        return true;

    QSize properRequestedSize = orientSize(requestedSize, orientation);

    if ((properRequestedSize.width() != 0 && image.width() >= properRequestedSize.width())
            || (properRequestedSize.height() != 0 && image.height() >= properRequestedSize.height())) {
        return true;
    }

    return false;
}
