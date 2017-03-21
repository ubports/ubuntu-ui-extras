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
#include "i18n.h"
#include "printers/printers.h"
#include "cupsdnotifier.h" // Note: this file was generated.

#include <QDBusConnection>
#include <QPrinterInfo>
#include <QQmlEngine>

Printers::Printers(QObject *parent)
    : Printers(new PrinterCupsBackend(new IppClient(), QPrinterInfo(),
        new OrgCupsCupsdNotifierInterface("", CUPSD_NOTIFIER_DBUS_PATH,
                                          QDBusConnection::systemBus())),
       parent)
{
}

Printers::Printers(PrinterBackend *backend, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
    , m_devices(backend)
    , m_drivers(backend)
    , m_model(backend)
    , m_jobs(backend)
{
    m_allPrinters.setSourceModel(&m_model);
    m_allPrinters.setSortRole(PrinterModel::Roles::DefaultPrinterRole);
    m_allPrinters.filterOnPdf(false);
    m_allPrinters.sort(0, Qt::DescendingOrder);

    m_allPrintersWithPdf.setSourceModel(&m_model);
    m_allPrintersWithPdf.setSortRole(PrinterModel::Roles::DefaultPrinterRole);
    m_allPrintersWithPdf.sort(0, Qt::DescendingOrder);

    // Let Qt be in charge of RAII.
    m_backend->setParent(this);

    connect(&m_drivers, SIGNAL(filterComplete()),
            this, SIGNAL(driverFilterChanged()));

    connect(&m_jobs, &QAbstractItemModel::rowsInserted, [this](
            const QModelIndex &parent, int first, int) {
        int jobId = m_jobs.data(m_jobs.index(first, 0, parent),
                                JobModel::Roles::IdRole).toInt();
        QString printerName = m_jobs.data(
            m_jobs.index(first, 0, parent),
            JobModel::Roles::PrinterNameRole
        ).toString();

        jobAdded(m_jobs.getJob(printerName, jobId));
    });

    // If the jobModel forces a refresh, load extended attributes for the job
    connect(&m_jobs, &JobModel::forceJobRefresh, [this](
            const QString &printerName, const int jobId) {
       jobAdded(m_jobs.getJob(printerName, jobId));
    });

    connect(&m_model, &QAbstractItemModel::rowsInserted, [this](
            const QModelIndex &parent, int first, int) {
        auto printer = m_model.data(
            m_model.index(first, 0, parent),
            PrinterModel::Roles::PrinterRole
        ).value<QSharedPointer<Printer>>();
        printerAdded(printer);
    });

    // Assign jobmodels to printers right away.
    for (int i = 0; i < m_model.rowCount(); i++) {
        printerAdded(m_model.data(
                m_model.index(i, 0),
                PrinterModel::Roles::PrinterRole
            ).value<QSharedPointer<Printer>>()
        );
    }

    // Ensure existing jobs have been added, incase some were added before
    // the connect to rowsInserted was done
    for (int i = 0; i < m_jobs.rowCount(); i++) {
        jobAdded(
            m_jobs.getJob(
                m_jobs.data(m_jobs.index(i), JobModel::Roles::PrinterNameRole).toString(),
                m_jobs.data(m_jobs.index(i), JobModel::IdRole).toInt()
            )
        );
    }

    if (m_backend->type() == PrinterEnum::PrinterType::CupsType) {
        ((PrinterCupsBackend*) m_backend)->createSubscription();
    }

    // Eagerly load the default printer.
    if (!m_backend->defaultPrinterName().isEmpty())
        m_backend->requestPrinter(m_backend->defaultPrinterName());
}

Printers::~Printers()
{
}

QAbstractItemModel* Printers::allPrinters()
{
    auto ret = &m_allPrinters;
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

QAbstractItemModel* Printers::allPrintersWithPdf()
{
    auto ret = &m_allPrintersWithPdf;
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

QAbstractItemModel* Printers::remotePrinters()
{
    /* Lazily initialize this model. Local printers are discerned from remotes
    by checking if they are remote. */
    if (!m_remotePrinters.sourceModel()) {
        m_remotePrinters.setSourceModel(&m_model);
        m_remotePrinters.filterOnRemote(true);
        m_remotePrinters.filterOnPdf(false);
        m_remotePrinters.invalidate();
        m_remotePrinters.sort(0, Qt::DescendingOrder);
    }

    auto ret = &m_remotePrinters;
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

QAbstractItemModel* Printers::localPrinters()
{
    /* Lazily initialize this model. Local printers are discerned from remotes
    by checking if they are remote. */
    if (!m_localPrinters.sourceModel()) {
        m_localPrinters.setSourceModel(&m_model);
        m_localPrinters.filterOnRemote(false);
        m_localPrinters.filterOnPdf(false);
        m_localPrinters.setSortRole(PrinterModel::Roles::DefaultPrinterRole);
        m_localPrinters.invalidate();
        m_localPrinters.sort(0, Qt::DescendingOrder);
    }

    auto ret = &m_localPrinters;
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

QAbstractItemModel* Printers::printJobs()
{
    auto ret = &m_jobs;
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

QAbstractItemModel* Printers::drivers()
{
    auto ret = &m_drivers;
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

QAbstractItemModel* Printers::devices()
{
    m_devices.load();
    auto ret = &m_devices;
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

QString Printers::driverFilter() const
{
    return m_drivers.filter();
}

void Printers::setDriverFilter(const QString &filter)
{
    m_drivers.setFilter(filter);
}

QString Printers::defaultPrinterName() const
{
    return m_backend->defaultPrinterName();
}

QString Printers::lastMessage() const
{
    return m_lastMessage;
}

PrinterJob* Printers::createJob(const QString &printerName)
{
    // Note: If called by QML, it gains ownership of this job.
    return new PrinterJob(printerName, m_backend);
}

QAbstractItemModel* Printers::createJobFilter()
{
    // Note: If called by QML, it gains ownership of the job filter.
    JobFilter *filter = new JobFilter();
    filter->setSourceModel(&m_jobs);

    filter->setSortRole(JobModel::Roles::CreationTimeRole);
    filter->sort(0, Qt::AscendingOrder);

    return filter;
}

void Printers::cancelJob(const QString &printerName, const int jobId)
{
    m_backend->cancelJob(printerName, jobId);
}

void Printers::holdJob(const QString &printerName, const int jobId)
{
    m_backend->holdJob(printerName, jobId);
}

void Printers::releaseJob(const QString &printerName, const int jobId)
{
    m_backend->releaseJob(printerName, jobId);
}

void Printers::setDefaultPrinterName(const QString &name)
{
    QString reply = m_backend->printerSetDefault(name);

    if (!reply.isEmpty()) {
        m_lastMessage = reply;
    }
}

void Printers::prepareToAddPrinter()
{
    if (m_drivers.rowCount() == 0) {
        m_drivers.load();
    }
}

void Printers::searchForDevices()
{
    m_devices.load();
}

bool Printers::addPrinter(const QString &name, const QString &ppd,
                          const QString &device, const QString &description,
                          const QString &location)
{
    QString reply = m_backend->printerAdd(name, device, ppd, description,
                                          location);
    if (!reply.isEmpty()) {
        m_lastMessage = reply;
        return false;
    }

    provisionPrinter(name);

    return true;
}

bool Printers::addPrinterWithPpdFile(const QString &name,
                                     const QString &ppdFileName,
                                     const QString &device,
                                     const QString &description,
                                     const QString &location)
{
    QString reply = m_backend->printerAddWithPpd(name, device, ppdFileName,
                                                 description, location);
    if (!reply.isEmpty()) {
        m_lastMessage = reply;
        return false;
    }

    provisionPrinter(name);

    return true;
}

void Printers::provisionPrinter(const QString &name)
{
    // We mimic what System Config Printer does here.
    m_backend->printerSetEnabled(name, true);
    m_backend->printerSetAcceptJobs(name, true);
}

bool Printers::removePrinter(const QString &name)
{
    QString reply = m_backend->printerDelete(name);

    if (!reply.isEmpty()) {
        m_lastMessage = reply;
        return false;
    }
    return true;
}

void Printers::jobAdded(QSharedPointer<PrinterJob> job)
{
    auto printer = m_model.getPrinterByName(job->printerName());

    // Check if we have a valid printer, does not need to be loaded as JobLoader
    // creates it's own Backend.
    if (printer && job) {
        // TODO: this printer may not be fully loaded
        // Which has the side affect of colorModel, duplex, quality not working
        // in PrinterJob as Printer::supportedColorModels etc fail
        // Potentially trigger loadPrinter and listen for the new printer?

        // Set the printer to the job
        m_jobs.updateJobPrinter(job, printer);

        // Trigger JobLoader to load extended attributes in the background
        m_backend->requestJobExtendedAttributes(printer, job);
    }
}

void Printers::printerAdded(QSharedPointer<Printer> printer)
{
    printer->setJobModel(&m_jobs);

    // Loop through jobs and associate a printer with it.
    for (int i = 0; i < m_jobs.rowCount(); i++) {
        QModelIndex idx = m_jobs.index(i, 0);

        QString printerName = m_jobs.data(
            idx, JobModel::Roles::PrinterNameRole
        ).toString();

        int jobId = m_jobs.data(idx, JobModel::Roles::IdRole).toInt();
        auto job = m_jobs.getJob(printerName, jobId);
        if (printerName == printer->name() && !job->printer()) {
            jobAdded(job);
        }
    }
}

void Printers::loadPrinter(const QString &name)
{
    auto printer = m_model.getPrinterByName(name);
    if (!printer) {
        qWarning() << Q_FUNC_INFO << "no known printer named" << name;
        return;
    }

    if (printer->type() == PrinterEnum::PrinterType::ProxyType) {
        m_backend->requestPrinter(name);
    }
}

void Printers::printTestPage(const QString &name)
{

    auto printer = m_model.getPrinterByName(name);
    if (!printer) {
        qWarning() << Q_FUNC_INFO << "no known printer named" << name;
        return;
    }

    auto page = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                       "cups/data/default-testpage.pdf",
                                       QStandardPaths::LocateFile);

    if (page.isEmpty()) {
        qCritical() << Q_FUNC_INFO << "Could not find test page.";
        return;
    }

    auto job = new PrinterJob(name, m_backend);
    job->setPrinter(printer);
    job->setTitle(__("Test page"));
    job->printFile(QUrl::fromLocalFile(page));
    job->deleteLater();
}
