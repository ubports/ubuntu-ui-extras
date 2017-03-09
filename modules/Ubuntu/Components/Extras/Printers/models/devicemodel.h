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

#ifndef USC_PRINTER_DEVICEMODEL_H
#define USC_PRINTER_DEVICEMODEL_H

#include "printers_global.h"

#include "structs.h"

#include <QAbstractListModel>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QVariant>

class PRINTERS_DECL_EXPORT DeviceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool searching MEMBER m_isSearching NOTIFY searchingChanged)
public:
    explicit DeviceModel(PrinterBackend *backend, QObject *parent = Q_NULLPTR);
    ~DeviceModel();

    enum Roles
    {
        // Qt::DisplayRole holds device name
        ClassRole = Qt::UserRole,
        IdRole,
        InfoRole,
        UriRole,
        LocationRole,
        TypeRole,
        MakeModelRole,
        LastRole = MakeModelRole,
    };

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

    int count() const;
    void load();
    void clear();

private Q_SLOTS:
    void deviceLoaded(const Device &device);
    void deviceSearchFinished();

Q_SIGNALS:
    void countChanged();
    void searchingChanged();

private:
    /* Checks if we want it in the list or not. Wanted is intentionally
    ambiguous. */
    bool deviceWanted(const Device &device);

    PrinterBackend *m_backend;
    QList<Device> m_devices;
    bool m_isSearching;
};

#endif // USC_PRINTER_DEVICEMODEL_H
