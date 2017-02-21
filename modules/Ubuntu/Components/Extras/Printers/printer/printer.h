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

#ifndef USC_PRINTERS_PRINTER_H
#define USC_PRINTERS_PRINTER_H

#include "printers_global.h"

#include "backend/backend.h"
#include "enums.h"
#include "models/jobmodel.h"
#include "printer/printerjob.h"
#include "structs.h"

#include <QObject>
#include <QPageSize>
#include <QList>
#include <QScopedPointer>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>

class PrinterBackend;
class PrinterJob;
class PRINTERS_DECL_EXPORT Printer : public QObject
{
    Q_OBJECT
public:
    explicit Printer(PrinterBackend *backend, QObject *parent = nullptr);
    ~Printer();

    bool enabled() const;
    ColorModel defaultColorModel() const;
    QList<ColorModel> supportedColorModels() const;
    QList<PrinterEnum::DuplexMode> supportedDuplexModes() const;
    QStringList supportedDuplexStrings() const;
    PrinterEnum::DuplexMode defaultDuplexMode() const;
    QString name() const;
    PrintQuality defaultPrintQuality() const;
    QList<PrintQuality> supportedPrintQualities() const;
    QString description() const;
    QPageSize defaultPageSize() const;
    QList<QPageSize> supportedPageSizes() const;
    PrinterEnum::AccessControl accessControl() const;
    PrinterEnum::ErrorPolicy errorPolicy() const;
    QStringList users() const;
    PrinterEnum::State state() const;
    QString lastStateMessage() const;
    bool acceptJobs() const;
    bool holdsDefinition() const;
    QAbstractItemModel* jobs();

    PrinterEnum::PrinterType type() const;

    void setAccessControl(const PrinterEnum::AccessControl &accessControl);
    void setDefaultColorModel(const ColorModel &colorModel);
    void setDescription(const QString &description);
    void setDefaultDuplexMode(const PrinterEnum::DuplexMode &duplexMode);
    void setEnabled(const bool enabled);
    void setAcceptJobs(const bool accepting);
    void setErrorPolicy(const PrinterEnum::ErrorPolicy &errorPolicy);
    void setDefaultPrintQuality(const PrintQuality &quality);
    void setDefaultPageSize(const QPageSize &pageSize);
    void setJobModel(JobModel* jobModel);

    bool deepCompare(QSharedPointer<Printer> other) const;
    void updateFrom(QSharedPointer<Printer> other);


public Q_SLOTS:
    // Add user that is either denied or allowed printer. See AccessControl.
    void addUser(const QString &username);

    int printFile(const QString &filepath, const PrinterJob *options);

    // Removes user. See addUser.
    void removeUser(const QString &username);

private:
    void loadAcceptJobs();
    void loadColorModel();
    void loadPrintQualities();

    JobFilter m_jobs;
    PrinterBackend *m_backend;
    ColorModel m_defaultColorModel;
    QList<ColorModel> m_supportedColorModels;
    PrintQuality m_defaultPrintQuality;
    QList<PrintQuality> m_supportedPrintQualities;
    bool m_acceptJobs;
};

// FIXME: not necessary outside tests
Q_DECLARE_METATYPE(QList<PrinterEnum::DuplexMode>)

#endif // USC_PRINTERS_PRINTER_H
