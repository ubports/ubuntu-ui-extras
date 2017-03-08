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

#include "cups/ippclient.h"
#include "devicesearcher.h"

DeviceSearcher::DeviceSearcher(IppClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
}

DeviceSearcher::~DeviceSearcher()
{
    delete m_client;
}

void DeviceSearcher::load()
{
    if (!m_client->getDevices(&DeviceSearcher::deviceCallBack, this)) {
        Q_EMIT failed(cupsLastErrorString());
    }
    Q_EMIT finished();
}

void DeviceSearcher::deviceCallBack(
    const char *deviceClass,
    const char *deviceId,
    const char *deviceInfo,
    const char *deviceMakeAndModel,
    const char *deviceUri,
    const char *deviceLocation,
    void *context)
{
    auto searcher = qobject_cast<DeviceSearcher*>(static_cast<QObject*>(context));
    if (!searcher) {
        qWarning() << Q_FUNC_INFO << "context not a DeviceSearcher.";
        return;
    }

    Device d;
    d.cls = deviceClass;
    d.id = deviceId;
    d.info = deviceInfo;
    d.makeAndModel = deviceMakeAndModel;
    d.uri = deviceUri;
    d.location = deviceLocation;

    searcher->deviceFound(d);
}

void DeviceSearcher::deviceFound(const Device &device)
{
    Q_EMIT loaded(device);
}
