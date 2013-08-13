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

#ifndef IMAGEFILEHACK_H
#define IMAGEFILEHACK_H

#include <QObject>

class ImageFileHack : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString account READ account NOTIFY accountChanged)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)

public:
    explicit ImageFileHack(QObject *parent = 0);
    QString account() { return m_account; }
    QString path() { return m_path; }

public Q_SLOTS:
    void read();

Q_SIGNALS:
    void accountChanged();
    void pathChanged();

private:
    QString m_account;
    QString m_path;
};

#endif // IMAGEFILEHACK_H
