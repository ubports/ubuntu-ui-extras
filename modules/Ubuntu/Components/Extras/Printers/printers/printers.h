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

#ifndef USC_PRINTERS_H
#define USC_PRINTERS_H

#include "printers_global.h"

#include "models/drivermodel.h"
#include "models/printermodel.h"
#include "printer/printer.h"

#include <QAbstractItemModel>
#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>

#define CUPSD_NOTIFIER_DBUS_PATH "/org/cups/cupsd/Notifier"

class PRINTERS_DECL_EXPORT Printers : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* allPrinters READ allPrinters CONSTANT)
    Q_PROPERTY(QAbstractItemModel* allPrintersWithPdf READ allPrintersWithPdf CONSTANT)
    Q_PROPERTY(QAbstractItemModel* recentPrinters READ recentPrinters CONSTANT)
    Q_PROPERTY(QAbstractItemModel* printJobs READ printJobs CONSTANT)
    Q_PROPERTY(QAbstractItemModel* drivers READ drivers CONSTANT)
    Q_PROPERTY (QString driverFilter READ driverFilter WRITE setDriverFilter NOTIFY driverFilterChanged)
    Q_PROPERTY(QString defaultPrinterName READ defaultPrinterName WRITE setDefaultPrinterName NOTIFY defaultPrinterNameChanged)
    Q_PROPERTY(QString lastMessage READ lastMessage CONSTANT)

public:
    explicit Printers(QObject *parent = Q_NULLPTR);

    // Note: Printers takes ownership of backend.
    explicit Printers(PrinterBackend *backend, QObject *parent = Q_NULLPTR);
    ~Printers();

    QAbstractItemModel* allPrinters();
    QAbstractItemModel* allPrintersWithPdf();
    QAbstractItemModel* recentPrinters();
    QAbstractItemModel* printJobs();
    QAbstractItemModel* drivers();
    QString driverFilter() const;
    QString defaultPrinterName() const;
    QString lastMessage() const;

    void setDefaultPrinterName(const QString &name);
    void setDriverFilter(const QString &filter);

public Q_SLOTS:
    QSharedPointer<Printer> getPrinterByName(const QString &name);
    QSharedPointer<Printer> getJobOwner(const int &jobId);

    PrinterJob* createJob(const QString &printerName);
    void cancelJob(const QString &printerName, const int jobId);

    /* Instructs us to start loading drivers and what have you. In most cases,
    the user is likely to merely configure existing printers/jobs. Loading
    (at least) 12.000 drivers isn't relevant to those scenarios, so in order to
    add printers, this method should be called first. */
    void prepareToAddPrinter();

    bool addPrinter(const QString &name, const QString &ppd,
                    const QString &device, const QString &description,
                    const QString &location);
    bool addPrinterWithPpdFile(const QString &name, const QString &ppdFileName,
                               const QString &device,
                               const QString &description,
                               const QString &location);

    bool removePrinter(const QString &name);

private Q_SLOTS:
    void jobAdded(QSharedPointer<PrinterJob> job);
    void printerAdded(QSharedPointer<Printer> printer);

Q_SIGNALS:
    void defaultPrinterNameChanged();
    void driverFilterChanged();

private:
    PrinterBackend *m_backend;
    DriverModel m_drivers;
    PrinterModel m_model;
    JobModel m_jobs;
    PrinterFilter m_allPrinters;
    PrinterFilter m_allPrintersWithPdf;
    PrinterFilter m_recentPrinters;
    QString m_lastMessage;
};

#endif // USC_PRINTERS_H
