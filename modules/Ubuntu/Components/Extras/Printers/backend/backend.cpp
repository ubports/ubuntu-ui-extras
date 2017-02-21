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

#include "backend/backend.h"

PrinterBackend::PrinterBackend(const QString &printerName, QObject *parent)
    : QObject(parent)
    , m_printerName(printerName)
    , m_type(PrinterEnum::PrinterType::ProxyType)
{
}

PrinterBackend::~PrinterBackend()
{
}

bool PrinterBackend::holdsDefinition() const
{
    return false;
}

QString PrinterBackend::printerAdd(const QString &name,
                                   const QString &uri,
                                   const QString &ppdFile,
                                   const QString &info,
                                   const QString &location)
{
    Q_UNUSED(name);
    Q_UNUSED(uri);
    Q_UNUSED(ppdFile);
    Q_UNUSED(info);
    Q_UNUSED(location);
    return QString();
}

QString PrinterBackend::printerAddWithPpd(const QString &name,
                                          const QString &uri,
                                          const QString &ppdFileName,
                                          const QString &info,
                                          const QString &location)
{
    Q_UNUSED(name);
    Q_UNUSED(uri);
    Q_UNUSED(ppdFileName);
    Q_UNUSED(info);
    Q_UNUSED(location);
    return QString();
}

QString PrinterBackend::printerDelete(const QString &name)
{
    Q_UNUSED(name);
    return QString();
}

QString PrinterBackend::printerSetDefault(const QString &name)
{
    Q_UNUSED(name);
    return QString();
}

QString PrinterBackend::printerSetEnabled(const QString &name,
                                          const bool enabled)
{
    Q_UNUSED(name);
    Q_UNUSED(enabled);
    return QString();
}

QString PrinterBackend::printerSetAcceptJobs(
    const QString &name,
    const bool enabled,
    const QString &reason)
{
    Q_UNUSED(name);
    Q_UNUSED(enabled);
    Q_UNUSED(reason);
    return QString();
}

QString PrinterBackend::printerSetInfo(const QString &name,
                                       const QString &info)
{
    Q_UNUSED(name);
    Q_UNUSED(info);
    return QString();
}

QString PrinterBackend::printerSetLocation(const QString &name,
                                           const QString &location)
{
    Q_UNUSED(name);
    Q_UNUSED(location);
    return QString();
}

QString PrinterBackend::printerSetShared(const QString &name,
                                         const bool shared)
{
    Q_UNUSED(name);
    Q_UNUSED(shared);
    return QString();
}

QString PrinterBackend::printerSetJobSheets(const QString &name,
                                            const QString &start,
                                            const QString &end)
{
    Q_UNUSED(name);
    Q_UNUSED(start);
    Q_UNUSED(end);
    return QString();
}

QString PrinterBackend::printerSetErrorPolicy(const QString &name,
                                              const PrinterEnum::ErrorPolicy &policy)
{
    Q_UNUSED(name);
    Q_UNUSED(policy);
    return QString();
}


QString PrinterBackend::printerSetOpPolicy(const QString &name,
                                           const PrinterEnum::OperationPolicy &policy)
{
    Q_UNUSED(name);
    Q_UNUSED(policy);
    return QString();
}

QString PrinterBackend::printerSetUsersAllowed(const QString &name,
                                               const QStringList &users)
{
    Q_UNUSED(name);
    Q_UNUSED(users);
    return QString();
}

QString PrinterBackend::printerSetUsersDenied(const QString &name,
                                              const QStringList &users)
{
    Q_UNUSED(name);
    Q_UNUSED(users);
    return QString();
}

QString PrinterBackend::printerAddOptionDefault(const QString &name,
                                                const QString &option,
                                                const QStringList &values)
{
    Q_UNUSED(name);
    Q_UNUSED(option);
    Q_UNUSED(values);
    return QString();
}

QString PrinterBackend::printerDeleteOptionDefault(const QString &name,
                                                   const QString &value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
    return QString();
}

QString PrinterBackend::printerAddOption(const QString &name,
                                         const QString &option,
                                         const QStringList &values)
{
    Q_UNUSED(name);
    Q_UNUSED(option);
    Q_UNUSED(values);
    return QString();
}

QVariant PrinterBackend::printerGetOption(const QString &name,
                                          const QString &option) const
{
    Q_UNUSED(name);
    Q_UNUSED(option);
    return QVariant();
}

QMap<QString, QVariant> PrinterBackend::printerGetOptions(
    const QString &name, const QStringList &options) const
{
    Q_UNUSED(name);
    Q_UNUSED(options);
    return QMap<QString, QVariant>();
}

// FIXME: maybe have a PrinterDest iface that has a CupsDest impl?
cups_dest_t* PrinterBackend::makeDest(const QString &name,
                              const PrinterJob *options)
{
    Q_UNUSED(name);
    Q_UNUSED(options);
    return Q_NULLPTR;
}

void PrinterBackend::cancelJob(const QString &name, const int jobId)
{
    Q_UNUSED(jobId);
    Q_UNUSED(name);
}

int PrinterBackend::printFileToDest(const QString &filepath,
                            const QString &title,
                            const cups_dest_t *dest)
{
    Q_UNUSED(filepath);
    Q_UNUSED(title);
    Q_UNUSED(dest);
    return -1;
}

QList<QSharedPointer<PrinterJob>> PrinterBackend::printerGetJobs()
{
    return QList<QSharedPointer<PrinterJob>>{};
}

QMap<QString, QVariant> PrinterBackend::printerGetJobAttributes(
        const QString &name, const int jobId)
{
    Q_UNUSED(name);
    Q_UNUSED(jobId);
    return QMap<QString, QVariant>();
}

QString PrinterBackend::printerName() const
{
    return m_printerName;
}

QString PrinterBackend::description() const
{
    return QString();
}

QString PrinterBackend::location() const
{
    return QString();
}

QString PrinterBackend::makeAndModel() const
{
    return QString();
}

PrinterEnum::State PrinterBackend::state() const
{
    return PrinterEnum::State::IdleState;
}

QList<QPageSize> PrinterBackend::supportedPageSizes() const
{
    return QList<QPageSize>();
}

QPageSize PrinterBackend::defaultPageSize() const
{
    return QPageSize();
}

bool PrinterBackend::supportsCustomPageSizes() const
{
    return false;
}

QPageSize PrinterBackend::minimumPhysicalPageSize() const
{
    return QPageSize();
}

QPageSize PrinterBackend::maximumPhysicalPageSize() const
{
    return QPageSize();
}

QList<int> PrinterBackend::supportedResolutions() const
{
    return QList<int>();
}

PrinterEnum::DuplexMode PrinterBackend::defaultDuplexMode() const
{
    return PrinterEnum::DuplexMode::DuplexNone;
}

QList<PrinterEnum::DuplexMode> PrinterBackend::supportedDuplexModes() const
{
    return QList<PrinterEnum::DuplexMode>();
}

QList<QSharedPointer<Printer>> PrinterBackend::availablePrinters()
{
    return QList<QSharedPointer<Printer>>();
}

QStringList PrinterBackend::availablePrinterNames()
{
    return QStringList();
}

QSharedPointer<Printer> PrinterBackend::getPrinter(const QString &printerName)
{
    Q_UNUSED(printerName);
    return QSharedPointer<Printer>(Q_NULLPTR);
}

QString PrinterBackend::defaultPrinterName()
{
    return QString();
}

void PrinterBackend::requestPrinterDrivers()
{
}

void PrinterBackend::requestPrinter(const QString &printerName)
{
    Q_UNUSED(printerName);
}

PrinterEnum::PrinterType PrinterBackend::type() const
{
    return m_type;
}

void PrinterBackend::setPrinterNameInternal(const QString &printerName)
{
    m_printerName = printerName;
}

void PrinterBackend::refresh()
{
}
