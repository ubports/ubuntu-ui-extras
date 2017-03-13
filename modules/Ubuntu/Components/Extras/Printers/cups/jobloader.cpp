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

#include "jobloader.h"

#include "backend/backend_cups.h"

#include "cups/ippclient.h"
#include "cupsdnotifier.h" // Note: this file was generated.

#include "printers/printers.h"

#include <QDBusConnection>
#include <QPrinterInfo>

class PrinterCupsBackend;
JobLoader::JobLoader(QSharedPointer<Printer> printer,
                     QSharedPointer<PrinterJob> printerJob,
                     PrinterBackend *backend,
                     QObject *parent)
    : QObject(parent)
    , m_backend(backend)
    , m_job(printerJob)
    // FIXME: if we are using our own Backend do we need printer? or only name?
    // or can we use the same Backend in a thread safe way?
    , m_printer(printer)
{
}

JobLoader::~JobLoader()
{
}

void JobLoader::load()
{
    // Use our own IppClient and Backend so that this works nicely threaded
    IppClient *client = new IppClient();
    OrgCupsCupsdNotifierInterface* notifier = new OrgCupsCupsdNotifierInterface(
                "", CUPSD_NOTIFIER_DBUS_PATH, QDBusConnection::systemBus());

    QPrinterInfo info = QPrinterInfo::printerInfo(m_printer->name());
    auto backend = new PrinterCupsBackend(client, info, notifier);

    QSharedPointer<Printer> printer = QSharedPointer<Printer>(new Printer(backend));

    QSharedPointer<PrinterJob> job = QSharedPointer<PrinterJob>(
        new PrinterJob(printer->name(), backend, m_job->jobId())
    );

    // Copy things that we don't set in extended attributes
    job->setImpressionsCompleted(m_job->impressionsCompleted());
    job->setState(m_job->state());
    job->setTitle(m_job->title());

    // Set the printer for this thread
    job->setPrinter(printer);

    // Load the extended attributes of the job
    job->loadDefaults();

    Q_EMIT loaded(m_job, job);
    Q_EMIT finished();
}
