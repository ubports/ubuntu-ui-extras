/*
 * Copyright (C) 2011 Canonical Ltd
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
 */

#include "photo-metadata.h"

#include <cstdio>
#include <QBuffer>

namespace {
const Orientation DEFAULT_ORIENTATION = TOP_LEFT_ORIGIN;
const char* EXIF_ORIENTATION_KEY = "Exif.Image.Orientation";
const char* EXIF_DATETIMEDIGITIZED_KEY = "Exif.Photo.DateTimeDigitized";
const char* EXPOSURE_TIME_KEYS[] = {
    "Exif.Photo.DateTimeOriginal",
    "Xmp.exif.DateTimeOriginal",
    "Xmp.xmp.CreateDate",
    "Exif.Photo.DateTimeDigitized",
    "Xmp.exif.DateTimeDigitized",
    "Exif.Image.DateTime"
};
const size_t NUM_EXPOSURE_TIME_KEYS = 6;
const char* EXIF_DATE_FORMATS[] = {
    "%d:%d:%d %d:%d:%d",

    // for Minolta DiMAGE E223 (colon, instead of space, separates day from
    // hour in exif)
    "%d:%d:%d:%d:%d:%d",

    // for Samsung NV10 (which uses a period instead of colons for the date
    // and two spaces between date and time)
    "%d.%d.%d  %d:%d:%d"
};
const size_t NUM_EXIF_DATE_FORMATS = 3;
const float THUMBNAIL_SCALE = 8.5;

const char* get_first_matched(const char* keys[], size_t n_keys,
                              const QSet<QString>& in) {
    for (size_t i = 0; i < n_keys; i++) {
        if (in.contains(keys[i]))
            return keys[i];
    }
    
    return NULL;
}

bool is_xmp_key(const char* key) {
    return (key != NULL) ? (std::strncmp("Xmp.", key, 4) == 0) : false;
}

bool is_exif_key(const char* key) {
    return (key != NULL) ? (std::strncmp("Exif.", key, 5) == 0) : false;
}

// caller should test if 's' could be successfully parsed by invoking the
// isValid() method on the returned QDateTime instance; if isValid() == false,
// 's' couldn't be parsed
QDateTime parse_xmp_date_string(const char* s) {
    return QDateTime::fromString(s, Qt::ISODate);
}

// caller should test if 's' could be successfully parsed by invoking the
// isValid() method on the returned QDateTime instance; if isValid() == false,
// 's' couldn't be parsed
QDateTime parse_exif_date_string(const char* s) {
    for (size_t i = 0; i < NUM_EXIF_DATE_FORMATS; i++) {
        int year, month, day, hour, minute, second;
        if (std::sscanf(s, EXIF_DATE_FORMATS[i], &year, &month, &day, &hour,
                        &minute, &second) == 6) {
            // no need to check year, month, day, hour, minute and second variables
            // for bogus values before using them -- if the values are bogus, the
            // resulting QDateTime will be invalid, which is exactly what we want
            return QDateTime(QDate(year, month, day), QTime(hour, minute, second));
        }
    }
    
    // the no argument QDateTime constructor produces an invalid QDateTime,
    // which is what we want
    return QDateTime();
}
} // namespace

/*!
 * \brief PhotoMetadata::PhotoMetadata
 * \param filepath
 */
PhotoMetadata::PhotoMetadata(const char* filepath)
    : m_fileSourceInfo(filepath)
{
    m_image = Exiv2::ImageFactory::open(filepath);
    m_image->readMetadata();
}

/*!
 * \brief PhotoMetadata::fromFile
 * \param filepath
 * \return
 */
PhotoMetadata* PhotoMetadata::fromFile(const char* filepath)
{
    PhotoMetadata* result = NULL;
    try {
        result = new PhotoMetadata(filepath);

        if (!result->m_image->good()) {
            qDebug("Invalid image metadata in %s", filepath);
            delete result;
            return NULL;
        }

        Exiv2::ExifData& exif_data = result->m_image->exifData();
        Exiv2::ExifData::const_iterator end = exif_data.end();
        for (Exiv2::ExifData::const_iterator i = exif_data.begin(); i != end; i++)
            result->m_keysPresent.insert(QString(i->key().c_str()));

        Exiv2::XmpData& xmp_data = result->m_image->xmpData();
        Exiv2::XmpData::const_iterator end1 = xmp_data.end();
        for (Exiv2::XmpData::const_iterator i = xmp_data.begin(); i != end1; i++)
            result->m_keysPresent.insert(QString(i->key().c_str()));

        return result;
    } catch (Exiv2::AnyError& e) {
        qDebug("Error loading image metadata: %s", e.what());
        delete result;
        return NULL;
    }
}

/*!
 * \brief PhotoMetadata::fromFile
 * \param file
 * \return
 */
PhotoMetadata* PhotoMetadata::fromFile(const QFileInfo &file)
{
    return PhotoMetadata::fromFile(file.absoluteFilePath().toStdString().c_str());
}

/*!
 * \brief PhotoMetadata::orientation
 * \return
 */
Orientation PhotoMetadata::orientation() const
{
    Exiv2::ExifData& exif_data = m_image->exifData();

    if (exif_data.empty())
        return DEFAULT_ORIENTATION;

    if (m_keysPresent.find(EXIF_ORIENTATION_KEY) == m_keysPresent.end())
        return DEFAULT_ORIENTATION;

    long orientation_code = exif_data[EXIF_ORIENTATION_KEY].toLong();
    if (orientation_code < MIN_ORIENTATION || orientation_code > MAX_ORIENTATION)
        return DEFAULT_ORIENTATION;

    return static_cast<Orientation>(orientation_code);
}

/*!
 * \brief PhotoMetadata::exposureTime
 * \return
 */
QDateTime PhotoMetadata::exposureTime() const
{
    const char* matched = get_first_matched(EXPOSURE_TIME_KEYS,
                                            NUM_EXPOSURE_TIME_KEYS, m_keysPresent);
    if (matched == NULL)
        return QDateTime();

    if (is_exif_key(matched))
        return parse_exif_date_string(m_image->exifData()[matched].toString().c_str());

    if (is_xmp_key(matched))
        return parse_xmp_date_string(m_image->xmpData()[matched].toString().c_str());

    // No valid/known tag for exposure date/time
    return QDateTime();
}

/*!
 * \brief PhotoMetadata::orientationCorrection
 * \return
 */
OrientationCorrection PhotoMetadata::orientationCorrection() const
{
    return OrientationCorrection::fromOrientation(orientation());
}

/*!
 * \brief PhotoMetadata::orientationTransform
 * \return
 */
QTransform PhotoMetadata::orientationTransform() const
{
    return orientationCorrection().toTransform();
}

/*!
 * \brief PhotoMetadata::setOrientation
 * \param orientation
 */
void PhotoMetadata::setOrientation(Orientation orientation)
{
    Exiv2::ExifData& exif_data = m_image->exifData();

    exif_data[EXIF_ORIENTATION_KEY] = orientation;

    if (!m_keysPresent.contains(EXIF_ORIENTATION_KEY))
        m_keysPresent.insert(EXIF_ORIENTATION_KEY);
}

/*!
 * \brief PhotoMetadata::setDateTimeDigitized
 * \param digitized
 */
void PhotoMetadata::setDateTimeDigitized(const QDateTime& digitized)
{
    try {
        if (!m_image->good()) {
            qDebug("Do not set DateTimeDigitized, invalid image metadata.");
            return;
        }
 
        Exiv2::ExifData& exif_data = m_image->exifData();
 
        exif_data[EXIF_DATETIMEDIGITIZED_KEY] = digitized.toString("yyyy:MM:dd hh:mm:ss").toStdString();
 
        if (!m_keysPresent.contains(EXIF_DATETIMEDIGITIZED_KEY))
            m_keysPresent.insert(EXIF_DATETIMEDIGITIZED_KEY);
 
    } catch (Exiv2::AnyError& e) {
        qDebug("Do not set DateTimeDigitized, error reading image metadata; %s", e.what());
        return;
    }
}

/*!
 * \brief PhotoMetadata::save
 * \return
 */
bool PhotoMetadata::save() const
{
    try {
        m_image->writeMetadata();
        return true;
    } catch (Exiv2::AnyError& e) {
        return false;
    }
}

void PhotoMetadata::copyTo(PhotoMetadata *other) const
{
    other->m_image->setMetadata(*m_image);
}

void PhotoMetadata::updateThumbnail(QImage image)
{
    QImage scaled = image.scaled(image.width() / THUMBNAIL_SCALE,
                                 image.height() / THUMBNAIL_SCALE);
    QBuffer jpeg;
    jpeg.open(QIODevice::WriteOnly);
    scaled.save(&jpeg, "jpeg");
    Exiv2::ExifThumb thumb(m_image->exifData());
    thumb.setJpegThumbnail((Exiv2::byte*) jpeg.data().constData(), jpeg.size());
}
