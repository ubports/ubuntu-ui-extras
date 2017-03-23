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

#ifndef USC_PRINTERS_CUPS_JOBLOADER_H
#define USC_PRINTERS_CUPS_JOBLOADER_H

#include "printer/printer.h"
#include "printer/printerjob.h"

#include <QList>
#include <QObject>
#include <QSharedPointer>

class JobLoader : public QObject
{
    Q_OBJECT
    PrinterBackend *m_backend;
    int m_job_id;
    QString m_printer_name;
public:
    explicit JobLoader(PrinterBackend *backend,
                       QString printerName, int jobId,
                       QObject *parent = Q_NULLPTR);
    ~JobLoader();

public Q_SLOTS:
    void load();

Q_SIGNALS:
    void finished();
    void loaded(QString, int, QMap<QString, QVariant>);
};

#endif // USC_PRINTERS_CUPS_JOBLOADER_H
