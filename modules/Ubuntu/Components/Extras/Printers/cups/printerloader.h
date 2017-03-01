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

#ifndef USC_PRINTERS_CUPS_PRINTERLOADER_H
#define USC_PRINTERS_CUPS_PRINTERLOADER_H

#include "cups/ippclient.h"
#include "cupsdnotifier.h" // Note: this file was generated.
#include "printer/printer.h"

#include <QList>
#include <QObject>
#include <QSharedPointer>

class PrinterLoader : public QObject
{
    Q_OBJECT
    const QString m_printerName;
    IppClient *m_client;
    OrgCupsCupsdNotifierInterface *m_notifier;
public:
    explicit PrinterLoader(const QString &printerName,
                           IppClient *client,
                           OrgCupsCupsdNotifierInterface* notifier,
                           QObject *parent = Q_NULLPTR);
    ~PrinterLoader();

public Q_SLOTS:
    void load();

Q_SIGNALS:
    void finished();
    void loaded(QSharedPointer<Printer> printer);
};

#endif // USC_PRINTERS_CUPS_PRINTERLOADER_H
