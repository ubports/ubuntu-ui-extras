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

#include "backend/backend_cups.h"
#include "models/devicemodel.h"

#include <QDebug>

DeviceModel::DeviceModel(PrinterBackend *backend, QObject *parent)
    : QAbstractListModel(parent)
    , m_backend(backend)
{
    connect(m_backend, SIGNAL(deviceFound(const Device&)),
            this, SLOT(deviceLoaded(const Device&)));
}

DeviceModel::~DeviceModel()
{
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_devices.size();
}

int DeviceModel::count() const
{
    return rowCount();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;

    if ((0 <= index.row()) && (index.row() < m_devices.size())) {

        auto device = m_devices[index.row()];

        switch (role) {
        case Qt::DisplayRole:
            ret = device.toString();
            break;
        case IdRole:
            ret = device.id;
            break;
        case InfoRole:
            ret = device.info;
            break;
        case UriRole:
            ret = device.uri;
            break;
        case LocationRole:
            ret = device.location;
            break;
        case MakeModelRole:
            ret = device.makeModel;
            break;
        }
    }

    return ret;
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    static QHash<int,QByteArray> names;

    if (Q_UNLIKELY(names.empty())) {
        names[Qt::DisplayRole] = "displayName";
        names[IdRole] = "id";
        names[InfoRole] = "info";
        names[UriRole] = "uri";
        names[LocationRole] = "location";
        names[MakeModelRole] = "makeModel";
    }

    return names;
}

void DeviceModel::deviceLoaded(const Device &device)
{
    if (!deviceWanted(device)) {
        return;
    }

    if (!m_devices.contains(device)) {
        int i = m_devices.size();
        beginInsertRows(QModelIndex(), i, i);
        m_devices.append(device);
        endInsertRows();

        Q_EMIT countChanged();
    }
}

bool DeviceModel::deviceWanted(const Device &device)
{
    auto parts = device.uri.split(":", QString::SkipEmptyParts);
    return parts.size() > 1;
}

void DeviceModel::clear()
{
    beginResetModel();
    m_devices.clear();
    endResetModel();
}
