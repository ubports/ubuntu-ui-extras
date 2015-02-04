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

#ifndef PHOTOUTILS_H
#define PHOTOUTILS_H

#include <QObject>

class FileUtils : public QObject
{
    Q_OBJECT
public:
    explicit FileUtils(QObject *parent = 0);

    Q_INVOKABLE bool createDirectory(QString path) const;
    Q_INVOKABLE bool removeDirectory(QString path, bool recursive = false) const;
    Q_INVOKABLE QString createTemporaryDirectory(QString pathTemplate) const;

    Q_INVOKABLE bool remove(QString path) const;
    Q_INVOKABLE bool copy(QString sourceFile, QString destinationFile) const;
    Q_INVOKABLE bool rename(QString sourceFile, QString destinationFile) const;

    Q_INVOKABLE QString parentDirectory(QString path) const;
    Q_INVOKABLE QString nameFromPath(QString path) const;

    Q_INVOKABLE bool exists(QString path) const;
};

#endif // PHOTOUTILS_H
