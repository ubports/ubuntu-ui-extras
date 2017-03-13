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
    , m_printer(printer)
{
}

JobLoader::~JobLoader()
{
}

void JobLoader::load()
{
    QSharedPointer<Printer> printer;
    PrinterBackend *backend;

    // FIXME: always build a new Printer and IppClient as it isn't thread safe
    if (true || m_printer->type() == PrinterEnum::PrinterType::ProxyType) {
        IppClient *client = new IppClient();
        OrgCupsCupsdNotifierInterface* notifier = new OrgCupsCupsdNotifierInterface(
                    "", CUPSD_NOTIFIER_DBUS_PATH, QDBusConnection::systemBus());
        QPrinterInfo info = QPrinterInfo::printerInfo(m_printer->name());

        backend = new PrinterCupsBackend(client, info, notifier);
        printer = QSharedPointer<Printer>(new Printer(backend));
    } else {
        backend = m_backend;
        printer = m_printer;
    }

    // Construct a job
    QSharedPointer<PrinterJob> job = QSharedPointer<PrinterJob>(
        new PrinterJob(m_printer->name(), backend, m_job->jobId())
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

    // If the given Printer was not loaded expose our loaded one
    // FIXME: for now skip this, need to check if having different
    // notifier/ippClient/backend could cause issues?
    if (false && m_printer->type() == PrinterEnum::PrinterType::ProxyType) {
        Q_EMIT printerLoaded(printer);
    }

    Q_EMIT finished();
}
