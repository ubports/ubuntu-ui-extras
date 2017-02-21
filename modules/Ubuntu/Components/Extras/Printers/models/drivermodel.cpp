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
#include "models/drivermodel.h"

#include <QDebug>
#include <QtConcurrent>

DriverModel::DriverModel(PrinterBackend *backend, QObject *parent)
    : QAbstractListModel(parent)
    , m_backend(backend)
{
    connect(m_backend, SIGNAL(printerDriversLoaded(const QList<PrinterDriver>&)),
            this, SLOT(printerDriversLoaded(const QList<PrinterDriver>&)));

    QObject::connect(&m_watcher,
                     &QFutureWatcher<PrinterDriver>::finished,
                     this,
                     &DriverModel::filterFinished);

}

DriverModel::~DriverModel()
{
    cancel();
}

int DriverModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_drivers.size();
}

int DriverModel::count() const
{
    return rowCount();
}

QVariant DriverModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;

    if ((0 <= index.row()) && (index.row() < m_drivers.size())) {

        auto driver = m_drivers[index.row()];

        switch (role) {
        case Qt::DisplayRole:
            ret = driver.toString();
            break;
        case NameRole:
            ret = driver.name;
            break;
        case DeviceIdRole:
            ret = driver.deviceId;
            break;
        case LanguageRole:
            ret = driver.language;
            break;
        case MakeModelRole:
            ret = driver.makeModel;
            break;
        }
    }

    return ret;
}

QHash<int, QByteArray> DriverModel::roleNames() const
{
    static QHash<int,QByteArray> names;

    if (Q_UNLIKELY(names.empty())) {
        names[Qt::DisplayRole] = "displayName";
        names[NameRole] = "name";
        names[DeviceIdRole] = "deviceId";
        names[LanguageRole] = "language";
        names[MakeModelRole] = "makeModel";
    }

    return names;
}

void DriverModel::setFilter(const QString& pattern)
{
    QList<QByteArray> needles;
    Q_FOREACH(const QString patternPart, pattern.toLower().split(" ")) {
        needles.append(patternPart.toUtf8());
    }
    QList<PrinterDriver> list;

    if (m_watcher.isRunning())
        m_watcher.cancel();

    if (pattern.isEmpty()) {
        setModel(m_originalDrivers);
        m_filter = pattern;
        return;
    }

    if (!m_filter.isEmpty() && !m_drivers.isEmpty() &&
            pattern.startsWith(m_filter))
        list = m_drivers; // search in the smaller list
    else
        list = m_originalDrivers; //search in the whole list

    m_filter = pattern;

    QFuture<PrinterDriver> future(QtConcurrent::filtered(list,
            [needles] (const PrinterDriver &driver) {
                QByteArray haystack = driver.makeModel.toLower();
                Q_FOREACH(const QByteArray needle, needles) {
                    if (!haystack.contains(needle)) {
                        return false;
                    }
                }
                return true;
            }
        )
    );

    Q_EMIT filterBegin();

    m_watcher.setFuture(future);
}

QString DriverModel::filter() const
{
    return m_filter;
}

void DriverModel::filterFinished()
{
    setModel(m_watcher.future().results());
}

void DriverModel::load()
{
    m_backend->requestPrinterDrivers();
}

void DriverModel::cancel()
{
    if (m_watcher.isRunning())
        m_watcher.cancel();
}

void DriverModel::printerDriversLoaded(const QList<PrinterDriver> &drivers)
{
    m_originalDrivers = drivers;
    setModel(m_originalDrivers);
}

void DriverModel::setModel(const QList<PrinterDriver> &drivers)
{
    beginResetModel();
    m_drivers = drivers;
    endResetModel();

    Q_EMIT filterComplete();
}
