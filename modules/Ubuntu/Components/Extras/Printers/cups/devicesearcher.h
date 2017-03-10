/*
 * Copyright (C) 2017 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef USC_PRINTERS_CUPS_DEVICESEARCHER_H
#define USC_PRINTERS_CUPS_DEVICESEARCHER_H

#include "cups/ippclient.h"
#include "structs.h"

#include <QObject>
#include <QString>

/* Class representing a device search. It is a worker object due to the time
such a search takes (minimum 10 seconds). */
class DeviceSearcher : public QObject
{
    Q_OBJECT
    IppClient *m_client;
public:
    explicit DeviceSearcher(IppClient *client = new IppClient,
                            QObject *parent = Q_NULLPTR);
    ~DeviceSearcher();

public Q_SLOTS:
    void load();

private:
    static void deviceCallBack(
        const char *cls,
        const char *id,
        const char *info,
        const char *makeAndModel,
        const char *uri,
        const char *location,
        void *context);
    void deviceFound(const Device &device);

Q_SIGNALS:
    void loaded(const Device &device);
    void failed(const QString &errorMessage);
    void finished();
};

#endif // USC_PRINTERS_CUPS_DEVICESEARCHER_H
