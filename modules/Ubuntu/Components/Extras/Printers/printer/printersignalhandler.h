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

#ifndef USC_PRINTER_SIGNAL_HANDLER_H
#define USC_PRINTER_SIGNAL_HANDLER_H

#include "printers_global.h"

#include <QObject>
#include <QSet>
#include <QTimer>

class PRINTERS_DECL_EXPORT PrinterSignalHandler : public QObject
{
    Q_OBJECT
    QTimer m_timer;
    QSet<QString> m_unprocessed;
public:
    explicit PrinterSignalHandler(int triggerEventDelay = 500,
                                  QObject *parent = Q_NULLPTR);
    ~PrinterSignalHandler();

public Q_SLOTS:
    void onPrinterModified(
        const QString &text, const QString &printerUri,
        const QString &printerName, uint printerState,
        const QString &printerStateReason, bool acceptingJobs
    );
    void onPrinterStateChanged(
        const QString &text, const QString &printerUri,
        const QString &printerName, uint printerState,
        const QString &printerStateReason, bool acceptingJobs
    );

private Q_SLOTS:
    void process();

Q_SIGNALS:
    void printerModified(const QString &printerName);
};

#endif // USC_PRINTER_SIGNAL_HANDLER_H
