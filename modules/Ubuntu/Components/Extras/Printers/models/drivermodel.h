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

#ifndef USC_PRINTER_DRIVERMODEL_H
#define USC_PRINTER_DRIVERMODEL_H

#include "printers_global.h"

#include "structs.h"

#include <QAbstractListModel>
#include <QFutureWatcher>
#include <QModelIndex>
#include <QObject>
#include <QVariant>

class PRINTERS_DECL_EXPORT DriverModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit DriverModel(PrinterBackend *backend, QObject *parent = Q_NULLPTR);
    ~DriverModel();

    enum Roles
    {
        // Qt::DisplayRole holds driver name
        NameRole = Qt::UserRole,
        DeviceIdRole,
        LanguageRole,
        MakeModelRole,
        LastRole = MakeModelRole,
    };

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

    int count() const;

    QString filter() const;
    void setFilter(const QString& pattern);

public Q_SLOTS:
    // Start loading the model.
    void load();

    // Cancel loading of the model.
    void cancel();

private Q_SLOTS:
    void printerDriversLoaded(const QList<PrinterDriver> &drivers);
    void filterFinished();

Q_SIGNALS:
    void countChanged();
    void filterBegin();
    void filterComplete();

private:
    void setModel(const QList<PrinterDriver> &drivers);
    PrinterBackend *m_backend;
    QList<PrinterDriver> m_drivers;
    QList<PrinterDriver> m_originalDrivers;
    QString m_filter;
    QFutureWatcher<PrinterDriver> m_watcher;
};

#endif // USC_PRINTER_DRIVERMODEL_H
