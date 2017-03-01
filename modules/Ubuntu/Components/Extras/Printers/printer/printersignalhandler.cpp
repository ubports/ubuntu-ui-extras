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

#include "printersignalhandler.h"

PrinterSignalHandler::PrinterSignalHandler(int triggerEventDelay,
                                           QObject *parent)
    : QObject(parent)
{
    m_timer.setInterval(triggerEventDelay);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(process()));
}

PrinterSignalHandler::~PrinterSignalHandler()
{
}

void PrinterSignalHandler::process()
{
    Q_FOREACH(auto printer, m_unprocessed) {
        Q_EMIT printerModified(printer);
    }
    m_unprocessed.clear();
    m_timer.stop();
}

void PrinterSignalHandler::onPrinterModified(
    const QString &text, const QString &printerUri,
    const QString &printerName, uint printerState,
    const QString &printerStateReason, bool acceptingJobs)
{

    Q_UNUSED(text);
    Q_UNUSED(printerUri);
    Q_UNUSED(printerState);
    Q_UNUSED(printerStateReason);
    Q_UNUSED(acceptingJobs);

    m_unprocessed << printerName;
    m_timer.start();
}

void PrinterSignalHandler::onPrinterStateChanged(
    const QString &text, const QString &printerUri,
    const QString &printerName, uint printerState,
    const QString &printerStateReason, bool acceptingJobs)
{
    Q_UNUSED(text);
    Q_UNUSED(printerUri);
    Q_UNUSED(printerState);
    Q_UNUSED(printerStateReason);
    Q_UNUSED(acceptingJobs);

    m_unprocessed << printerName;
    m_timer.start();
}
