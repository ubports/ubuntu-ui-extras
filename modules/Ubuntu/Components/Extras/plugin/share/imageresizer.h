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

#ifndef IMAGERESIZER_H
#define IMAGERESIZER_H

#include <QObject>
#include <QTemporaryFile>

/**
 * @brief Create a temporary rescaled copy of an image
 *
 * Create a temporary copy of the image in \a file. If \a largestSide
 * is smaller than the larger side of the image, then the image is
 * scaled down preserving aspect ratio so that the largest side matches.
 * Quality of the copy will be 85 unless you specify another \a quality
 * value (clamped to the 0-100 range).
 *
 * The path of the copy can be found in \a resizedFile. The copy is
 * automatically deleted when the \a ImageResizer object is destroyed, or
 * when any of the parameters change.
 */
class ImageResizer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString file READ file WRITE setFile NOTIFY fileChanged)
    Q_PROPERTY(QString resizedFile READ resizedFile NOTIFY resizedFileChanged)
    Q_PROPERTY(int largestSide READ largestSide WRITE setLargestSide NOTIFY largestSideChanged)
    Q_PROPERTY(int quality READ quality WRITE setQuality NOTIFY qualityChanged)

public:
    explicit ImageResizer(QObject *parent = 0);
    QString file() const;
    QString resizedFile() const;
    int largestSide() const;
    int quality() const;

    void setFile(const QString& file);
    void setLargestSide(int largestSide);
    void setQuality(int quality);

Q_SIGNALS:
    void fileChanged();
    void resizedFileChanged();
    void largestSideChanged();
    void qualityChanged();

private:
    void resizeImage();

    QString m_file;
    QTemporaryFile* m_resizedImage;
    int m_largestSide;
    int m_quality;
};

#endif // IMAGERESIZER_H
