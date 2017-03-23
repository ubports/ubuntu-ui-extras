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

#include "backend/backend.h"

#include "printers/printers.h"

#include <QDBusConnection>
#include <QPrinterInfo>

class PrinterCupsBackend;
JobLoader::JobLoader(PrinterBackend *backend,
                     QString printerName,
                     int jobId,
                     QObject *parent)
    : QObject(parent)
    , m_backend(backend)
    , m_job_id(jobId)
    , m_printer_name(printerName)
{
}

JobLoader::~JobLoader()
{
}

void JobLoader::load()
{
    QMap<QString, QVariant> map = m_backend->printerGetJobAttributes(
        m_printer_name, m_job_id
    );

    Q_EMIT loaded(m_printer_name, m_job_id, map);
    Q_EMIT finished();
}
