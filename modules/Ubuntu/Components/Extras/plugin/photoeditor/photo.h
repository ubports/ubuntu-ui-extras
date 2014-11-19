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
 * Ugo Riboni <ugo.riboni@canonical.com>
 */

#ifndef PHOTO_H_
#define PHOTO_H_

// util
#include "orientation.h"

// QT
#include <QFileInfo>

class PhotoEditCommand;
class PhotoEditThread;
class EditStack;

/*!
 * \brief The Photo class
 */
class Photo : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(int orientation READ orientation NOTIFY orientationChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit Photo();
    virtual ~Photo();

    static bool isValid(const QFileInfo& file);

    QString path() const;
    void setPath(QString path);
    QFileInfo file() const;
    bool busy() const;

    virtual QImage image(bool respectOrientation, const QSize &scaleSize=QSize());
    virtual Orientation orientation() const;

    Q_INVOKABLE void refreshFromDisk();
    Q_INVOKABLE void rotateRight();
    Q_INVOKABLE void autoEnhance();
    Q_INVOKABLE void exposureCompensation(qreal value);
    Q_INVOKABLE void colorBalance(qreal brightness, qreal contrast, qreal saturation, qreal hue);
    Q_INVOKABLE void crop(QVariant vrect);

    void setOrientation(Orientation orientation);
    const QSize &originalSize();
    Orientation originalOrientation() const;

    const QString &fileFormat() const;
    bool fileFormatHasMetadata() const;
    bool fileFormatHasOrientation() const;

Q_SIGNALS:
    void pathChanged();
    void orientationChanged();
    void busyChanged();

    void editFinished();
    void dataChanged();

private Q_SLOTS:
    void finishEditing();

private:
    QSize originalSize(Orientation orientation);
    void asyncEdit(const PhotoEditCommand& state);

    QString m_fileFormat;
    PhotoEditThread *m_editThread;
    QFileInfo m_file;
    bool m_busy;

    // We cache this data to avoid an image read at various times.
    QSize m_originalSize;
    Orientation m_orientation;
    Orientation m_originalOrientation;
};

#endif  // PHOTO_H_
