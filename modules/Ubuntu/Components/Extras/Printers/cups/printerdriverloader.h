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

#ifndef USC_PRINTERS_CUPS_DRIVERLOADER_H
#define USC_PRINTERS_CUPS_DRIVERLOADER_H

#include "ippclient.h"
#include "structs.h"

#include <QObject>
#include <QString>
#include <QStringList>

class PrinterDriverLoader : public QObject
{
    Q_OBJECT
public:
    PrinterDriverLoader(
        const QString &deviceId = "",
        const QString &language = "",
        const QString &makeModel = "",
        const QString &product = "",
        const QStringList &includeSchemes = QStringList(),
        const QStringList &excludeSchemes = QStringList());
    ~PrinterDriverLoader();

public Q_SLOTS:
    void process();
    void cancel();

Q_SIGNALS:
    void finished();
    void loaded(const QList<PrinterDriver> &drivers);
    void error(const QString &error);

private:
    QString m_deviceId = QString::null;
    QString m_language = QString::null;
    QString m_makeModel = QString::null;
    QString m_product = QString::null;
    QStringList m_includeSchemes;
    QStringList m_excludeSchemes;

    bool m_running = false;
    IppClient client;
};

#endif // USC_PRINTERS_CUPS_DRIVERLOADER_H
