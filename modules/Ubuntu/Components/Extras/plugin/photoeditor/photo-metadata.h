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
 */

#ifndef PHOTO_METADATA_H
#define PHOTO_METADATA_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QFutureWatcher>
#include <QtGui/QImage>

#include <exiv2/exiv2.hpp>

enum Orientation {
    ORIGINAL_ORIENTATION = 0,
    MIN_ORIENTATION = 1,
    TOP_LEFT_ORIGIN = 1,
    TOP_RIGHT_ORIGIN = 2,
    BOTTOM_RIGHT_ORIGIN = 3,
    BOTTOM_LEFT_ORIGIN = 4,
    LEFT_TOP_ORIGIN = 5,
    RIGHT_TOP_ORIGIN = 6,
    RIGHT_BOTTOM_ORIGIN = 7,
    LEFT_BOTTOM_ORIGIN = 8,
    MAX_ORIENTATION = 8
};
namespace PhotoEditor {

class PhotoMetadata : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(bool saving READ saving NOTIFY savingChanged)

public:
    PhotoMetadata();

    QString fileName() const;
    int orientation() const;
    bool saving() const;

    void setFileName(QString fileName);
    void setOrientation(int orientation);

    Q_INVOKABLE void saveImage(QImage image);
    Q_INVOKABLE void save();

Q_SIGNALS:
    void fileNameChanged();
    void orientationChanged();
    void savingChanged();

protected:
    void updateThumbnail(QImage image);
    void doSaveImage(QImage image);
    void doSave();

private:
    Exiv2::Image::AutoPtr m_image;
    QString m_fileName;
    QFutureWatcher<void> m_saveWatcher;
};
}
#endif // PHOTO_METADATA_H
