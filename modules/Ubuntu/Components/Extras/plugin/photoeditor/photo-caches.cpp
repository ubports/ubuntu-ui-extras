/*
 * Copyright (C) 2012 Canonical Ltd
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
 * Charles Lindsay <chaz@yorba.org>
 */

#include "photo-caches.h"

#include <QDir>
#include <utime.h>

const QString PhotoCaches::ORIGINAL_DIR = ".original";
const QString PhotoCaches::ENHANCED_DIR = ".enhanced";

/*!
 * \brief PhotoCaches::PhotoCaches
 * \param file
 */
PhotoCaches::PhotoCaches(const QFileInfo& file) : m_file(file),
    m_originalFile(file.dir(),
                   QString("%1/%2").arg(ORIGINAL_DIR).arg(file.fileName())),
    m_enhancedFile(file.dir(),
                   QString("%1/%2").arg(ENHANCED_DIR).arg(file.fileName()))
{
    // We always want our file checks to hit the disk.
    m_file.setCaching(false);
    m_originalFile.setCaching(false);
    m_enhancedFile.setCaching(false);
}

/*!
 * \brief PhotoCaches::hasCachedOriginal
 * \return
 */
bool PhotoCaches::hasCachedOriginal() const
{
    return m_originalFile.exists();
}

/*!
 * \brief PhotoCaches::hasCachedEnhanced
 * \return
 */
bool PhotoCaches::hasCachedEnhanced() const
{
    return m_enhancedFile.exists();
}

/*!
 * \brief PhotoCaches::originalFile
 * \return
 */
const QFileInfo& PhotoCaches::originalFile() const
{
    return m_originalFile;
}

/*!
 * \brief PhotoCaches::enhancedFile
 * \return
 */
const QFileInfo& PhotoCaches::enhancedFile() const
{
    return m_enhancedFile;
}

/*!
 * \brief PhotoCaches::pristineFile
 * Returns original_file() if it exists; otherwise, returns the file passed
 * to the constructor.
 * \return
 */
const QFileInfo& PhotoCaches::pristineFile() const
{
    return (hasCachedOriginal() ? m_originalFile : m_file);
}

/*!
 * \brief PhotoCaches::cacheOriginal
 * Moves the pristine file into .original so we don't mess it up.  Note that
 * this potentially removes the main file, so it must be followed by a copy
 * from original (or elsewhere) back to the file.
 * \return
 */
bool PhotoCaches::cacheOriginal()
{
    if (hasCachedOriginal()) {
        return true;
    }

    m_file.dir().mkdir(ORIGINAL_DIR);

    return rename(m_file, m_originalFile);
}

/*!
 * \brief PhotoCaches::restoreOriginal
 * Moves the file out of .original, overwriting the main file.  Note that
 * this removes the .original file.
 * \return
 */
bool PhotoCaches::restoreOriginal()
{
    if (!hasCachedOriginal()) {
        return true;
    }

    remove(m_file);
    // touch the file so that the thumbnails will correctly regenerate
    utime(m_originalFile.absoluteFilePath().toUtf8(), NULL);
    return rename(m_originalFile, m_file);
}

/*!
 * \brief PhotoCaches::cacheEnhancedFromOriginal
 * Copies the file in .original to .enhanced so it can then be enhanced.
 * \return
 */
bool PhotoCaches::cacheEnhancedFromOriginal()
{
    m_file.dir().mkdir(ENHANCED_DIR);

    // If called subsequently, the previously cached version is replaced.
    remove(m_enhancedFile);
    return copy(pristineFile(), m_enhancedFile);
}

/*!
 * \brief PhotoCaches::overwriteFromCache
 * Tries to overwrite the file from one of its cached versions.
 * \param preferEnhanced
 * \return
 */
bool PhotoCaches::overwriteFromCache(bool preferEnhanced)
{
    if (preferEnhanced && hasCachedEnhanced()) {
        remove(m_file);
        return copy(m_enhancedFile, m_file);
    } else if (hasCachedOriginal()) {
        remove(m_file);
        return copy(m_originalFile, m_file);
    } else {
        return true;
    }
}

/*!
 * \brief PhotoCaches::discardCachedOriginal
 */
void PhotoCaches::discardCachedOriginal()
{
    remove(m_originalFile);
}

/*!
 * \brief PhotoCaches::discardCachedEnhanced
 */
void PhotoCaches::discardCachedEnhanced()
{
    remove(m_enhancedFile);
}

/*!
 * \brief PhotoCaches::discardAll
 */
void PhotoCaches::discardAll()
{
    discardCachedOriginal();
    discardCachedEnhanced();
}
