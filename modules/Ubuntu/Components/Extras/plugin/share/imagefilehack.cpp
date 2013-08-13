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

#include "imagefilehack.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>

ImageFileHack::ImageFileHack(QObject *parent) :
    QObject(parent)
{
    QString m_account;
    QString m_path;
}

void ImageFileHack::read()
{
    QFileInfo imageFilePath(QDir::tempPath() + QDir::separator() + "sharelocation");
    if (imageFilePath.isFile()) {
        QFile imageFile(imageFilePath.absoluteFilePath());
        if (imageFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&imageFile);
            m_account = stream.readLine();
            qDebug () << "ACCOUNT: " << m_account;
            m_path = stream.readLine();
            qDebug () << "PATH: " << m_path;
            Q_EMIT accountChanged();
            Q_EMIT pathChanged();
        } else {
            qWarning() << "Failed to open share path file for reading: " << imageFilePath.absoluteFilePath();
        }
    } else {
        qWarning() << "Share path file doesn't exist: " << imageFilePath.absoluteFilePath();
    }
}

