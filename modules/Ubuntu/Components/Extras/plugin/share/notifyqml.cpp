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

#include <glib-object.h>
#include <libnotify/notify.h>

#include <QProcess>
#include <QStringList>

#include "notifyqml.h"

Notify::Notify(QObject *parent)
{
}

Notify::~Notify()
{
    notify_uninit();
}

QString Notify::appName() const
{
    return (!notify_is_initted()) ? "" : QString::fromUtf8(notify_get_app_name());
}

void Notify::setAppName(const QString &appName)
{
    if (!notify_is_initted()) {
        QByteArray data = appName.toUtf8();
        notify_init(data.data());
        Q_EMIT appNameChanged();
    } else {
        if (appName != QString::fromUtf8(notify_get_app_name())) {
            QByteArray data = appName.toUtf8();
            notify_set_app_name(data.data());
            Q_EMIT appNameChanged();
        }
    }
}

void Notify::notify(const QString &title, const QString &message, const QString &icon)
{
    QByteArray titleData = title.toUtf8();
    QByteArray messageData = message.toUtf8();
    QByteArray iconData = icon.toUtf8();
    NotifyNotification* notification = notify_notification_new(titleData.data(), messageData.data(), iconData.data());
    notify_notification_show(notification, NULL);
    g_object_unref(notification);
}
