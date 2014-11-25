/*
 * Copyright (C) 2014 Canonical Ltd
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

#include "file-utils.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

FileUtils::FileUtils(QObject *parent) :
    QObject(parent)
{
}

bool FileUtils::createDirectory(QString path) const
{ 
    if (path.isEmpty()) return false;

    return QDir(path).mkpath(".");
}

QString FileUtils::createTemporaryDirectory(QString pathTemplate) const
{
    QTemporaryDir dir(pathTemplate);
    if (!dir.isValid()) return QString();

    dir.setAutoRemove(false);
    return dir.path();
}

bool FileUtils::removeDirectory(QString path, bool recursive) const
{
    if (path.isEmpty()) return false;

    QDir dir(path);
    return (recursive) ? dir.removeRecursively() : dir.rmdir(".");
}

bool FileUtils::remove(QString path) const
{
    if (path.isEmpty()) return false;
    return QFile::remove(path);
}

bool FileUtils::copy(QString sourceFile, QString destinationFile) const
{
    if (sourceFile.isEmpty() || destinationFile.isEmpty()) return false;

    if (QFileInfo(destinationFile).exists()) QFile::remove(destinationFile);
    return QFile::copy(sourceFile, destinationFile);
}

bool FileUtils::rename(QString sourceFile, QString destinationFile) const
{
    if (sourceFile.isEmpty() || destinationFile.isEmpty()) return false;

    if (QFileInfo(destinationFile).exists()) QFile::remove(destinationFile);
    return QFile::rename(sourceFile, destinationFile);
}

QString FileUtils::parentDirectory(QString path) const
{
    if (QFileInfo(path).isDir()) {
        QDir dir(path);
        dir.cdUp();
        return dir.absolutePath();
    } else {
        return QFileInfo(path).dir().absolutePath();
    }
}

QString FileUtils::nameFromPath(QString path) const
{
    QString name = QFileInfo(path).fileName();
    if (name.isEmpty()) name = QDir(path).dirName();
    return name;
}

bool FileUtils::exists(QString path) const
{
    return QFileInfo::exists(path);
}
