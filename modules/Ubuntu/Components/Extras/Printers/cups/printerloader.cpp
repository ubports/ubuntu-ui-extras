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

#include "backend/backend_pdf.h"
#include "backend/backend_cups.h"
#include "printerloader.h"

#include <QPrinterInfo>

class PrinterCupsBackend;
PrinterLoader::PrinterLoader(const QString &printerName,
                             IppClient *client,
                             OrgCupsCupsdNotifierInterface* notifier,
                             QObject *parent)
    : QObject(parent)
    , m_printerName(printerName)
    , m_client(client)
    , m_notifier(notifier)
{
}

PrinterLoader::~PrinterLoader()
{
}

void PrinterLoader::load()
{
    QPrinterInfo info = QPrinterInfo::printerInfo(m_printerName);
    auto backend = new PrinterCupsBackend(m_client, info, m_notifier);

    // Dest or PPD was null, but we know it's name so we will use it.
    if (info.printerName().isEmpty()) {
        backend->setPrinterNameInternal(m_printerName);
    }

    auto p = QSharedPointer<Printer>(new Printer(backend));

    Q_EMIT loaded(p);
    Q_EMIT finished();
}
