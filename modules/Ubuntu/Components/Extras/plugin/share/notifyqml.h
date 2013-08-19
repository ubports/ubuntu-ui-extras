/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
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

#ifndef NOTIFYQML_H
#define NOTIFYQML_H

#include <QObject>

class Notify : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName WRITE setAppName NOTIFY appNameChanged)

public:
    explicit Notify(QObject *parent = 0);
    ~Notify();
    QString appName() const;
    void setAppName(const QString& appName);

Q_SIGNALS:
    void appNameChanged();

public Q_SLOTS:
    void notify(const QString& title, const QString& message, const QString& icon = QString());
};

#endif // NOTIFYQML_H
