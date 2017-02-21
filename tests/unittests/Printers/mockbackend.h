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

#ifndef USC_PRINTERS_MOCK_BACKEND_H
#define USC_PRINTERS_MOCK_BACKEND_H

#include "backend/backend.h"
#include "utils.h"

class MockPrinterBackend : public PrinterBackend
{
    Q_OBJECT
public:
    explicit MockPrinterBackend(QObject *parent = Q_NULLPTR)
        : MockPrinterBackend(QString::null, parent)
    {

    };

    explicit MockPrinterBackend(const QString &printerName,
                                QObject *parent = Q_NULLPTR)
      : PrinterBackend(printerName, parent)
    {
    };

    virtual ~MockPrinterBackend() {};

    virtual bool holdsDefinition() const override
    {
        return true;
    }

    virtual QString printerAdd(const QString &name,
                               const QString &uri,
                               const QString &ppdFile,
                               const QString &info,
                               const QString &location) override
    {
        Q_UNUSED(name);
        Q_UNUSED(uri);
        Q_UNUSED(ppdFile);
        Q_UNUSED(info);
        Q_UNUSED(location);
        return returnValue;
    }

    virtual QString printerAddWithPpd(const QString &name,
                                      const QString &uri,
                                      const QString &ppdFileName,
                                      const QString &info,
                                      const QString &location) override
    {
        Q_UNUSED(name);
        Q_UNUSED(uri);
        Q_UNUSED(ppdFileName);
        Q_UNUSED(info);
        Q_UNUSED(location);
        return returnValue;
    }

    virtual QString printerDelete(const QString &name) override
    {
        Q_FOREACH(QSharedPointer<Printer> p, m_availablePrinters) {
            if (p->name() == name) {
                m_availablePrinters.removeOne(p);
                break;
            }
        }
        return returnValue;
    }

    virtual QString printerSetDefault(const QString &name) override
    {
        Q_UNUSED(name);
        return returnValue;
    }

    virtual QString printerSetEnabled(const QString &name,
                                      const bool enabled) override
    {
        enableds.insert(name, enabled);
        return returnValue;
    }

    virtual QString printerSetAcceptJobs(
        const QString &name,
        const bool accept,
        const QString &reason = QString::null) override
    {
        Q_UNUSED(reason);
        printerOptions[name].insert("AcceptJobs", accept);
        return returnValue;
    }

    virtual QString printerSetInfo(const QString &name,
                                   const QString &info) override
    {
        infos.insert(name, info);
        return returnValue;
    }

    virtual QString printerSetLocation(const QString &name,
                                       const QString &location) override
    {
        locations.insert(name, location);
        return returnValue;
    }

    virtual QString printerSetShared(const QString &name,
                                     const bool shared) override
    {
        shareds.insert(name, shared);
        return returnValue;
    }

    virtual QString printerSetJobSheets(const QString &name,
                                        const QString &start,
                                        const QString &end) override
    {
        Q_UNUSED(name);
        Q_UNUSED(start);
        Q_UNUSED(end);
        return returnValue;
    }

    virtual QString printerSetErrorPolicy(const QString &name,
                                          const PrinterEnum::ErrorPolicy &policy) override
    {
        errorPolicies.insert(name, policy);
        return returnValue;
    }


    virtual QString printerSetOpPolicy(const QString &name,
                                       const PrinterEnum::OperationPolicy &policy) override
    {
        operationPolicies.insert(name, policy);
        return returnValue;
    }

    virtual QString printerSetUsersAllowed(const QString &name,
                                           const QStringList &users) override
    {
        printerOptions[name].insert("users-allowed", QVariant::fromValue(users));
        return returnValue;
    }

    virtual QString printerSetUsersDenied(const QString &name,
                                          const QStringList &users) override
    {
        printerOptions[name].insert("users-denied", QVariant::fromValue(users));
        return returnValue;
    }

    virtual QString printerAddOptionDefault(const QString &name,
                                            const QString &option,
                                            const QStringList &values) override
    {
        Q_UNUSED(name);
        Q_UNUSED(option);
        Q_UNUSED(values);
        return returnValue;
    }

    virtual QString printerDeleteOptionDefault(const QString &name,
                                               const QString &value) override
    {
        Q_UNUSED(name);
        Q_UNUSED(value);
        return returnValue;
    }

    virtual QString printerAddOption(const QString &name,
                                     const QString &option,
                                     const QStringList &values) override
    {
        printerOptions[name].insert(option, values);
        return returnValue;
    }


    // TODO: const for both these getters (if possible)!
    virtual QVariant printerGetOption(const QString &name,
                                      const QString &option) const override
    {
        // FIXME: if we're to return a variant, check that it can be converted
        return printerOptions[name].value(option);
    }

    virtual QMap<QString, QVariant> printerGetOptions(
        const QString &name, const QStringList &options) const override
    {
        QMap<QString, QVariant> opts;
        Q_FOREACH(const QString &option, options) {
            opts[option] = printerGetOption(name, option);
        }
        return opts;
    }

    // FIXME: maybe have a PrinterDest iface that has a CupsDest impl?
    virtual cups_dest_t* makeDest(const QString &name,
                                  const PrinterJob *options) override
    {
        Q_UNUSED(name);
        Q_UNUSED(options);
        return Q_NULLPTR;
    }

    virtual void cancelJob(const QString &name, const int jobId) override
    {
        Q_UNUSED(name);
        Q_UNUSED(jobId);
    }

    virtual int printFileToDest(const QString &filepath,
                                const QString &title,
                                const cups_dest_t *dest) override
    {
        Q_UNUSED(filepath);
        Q_UNUSED(title);
        Q_UNUSED(dest);
        return -1;
    }

    virtual QList<QSharedPointer<PrinterJob>> printerGetJobs() override
    {
        return m_jobs;
    }

    virtual QMap<QString, QVariant> printerGetJobAttributes(
            const QString &name, const int jobId) override
    {
        Q_UNUSED(name);
        Q_UNUSED(jobId);
        return QMap<QString, QVariant>();
    }

    virtual QString printerName() const override
    {
        return m_printerName;
    }

    virtual QString description() const override
    {
        return m_description;
    }

    virtual QString location() const override
    {
        return m_location;
    }

    virtual QString makeAndModel() const override
    {
        return m_makeAndModel;
    }

    virtual PrinterEnum::State state() const override
    {
        return m_state;
    }

    virtual QList<QPageSize> supportedPageSizes() const override
    {
        return m_supportedPageSizes;
    }

    virtual QPageSize defaultPageSize() const override
    {
        return m_defaultPageSize;
    }

    virtual bool supportsCustomPageSizes() const override
    {
        return false;
    }


    virtual QPageSize minimumPhysicalPageSize() const override
    {
        return QPageSize();
    }

    virtual QPageSize maximumPhysicalPageSize() const override
    {
        return QPageSize();
    }

    virtual QList<int> supportedResolutions() const override
    {
        return QList<int>{};
    }

    virtual PrinterEnum::DuplexMode defaultDuplexMode() const override
    {
        auto ppdMode = printerGetOption(printerName(), "Duplex").toString();
        return Utils::ppdChoiceToDuplexMode(ppdMode);
    }

    virtual QList<PrinterEnum::DuplexMode> supportedDuplexModes() const override
    {
        return m_supportedDuplexModes;
    }

    virtual QList<QSharedPointer<Printer>> availablePrinters() override
    {
        return m_availablePrinters;
    }

    virtual QStringList availablePrinterNames() override
    {
        return m_availablePrinterNames;
    }

    virtual QSharedPointer<Printer> getPrinter(const QString &printerName) override
    {
        Q_FOREACH(auto p, m_availablePrinters) {
            if (p->name() == printerName) {
                return p;
            }
        }
        return QSharedPointer<Printer>(Q_NULLPTR);
    }

    virtual QString defaultPrinterName() override
    {
        return m_defaultPrinterName;
    }

    void mockPrinterAdded(
        const QString &text,
        const QString &printerUri,
        const QString &printerName,
        uint printerState,
        const QString &printerStateReason,
        bool acceptingJobs
    )
    {
        Q_EMIT printerAdded(text, printerUri, printerName, printerState, printerStateReason, acceptingJobs);
    }

    void mockPrinterModified(
        const QString &text,
        const QString &printerUri,
        const QString &printerName,
        uint printerState,
        const QString &printerStateReason,
        bool acceptingJobs
    )
    {
        Q_EMIT printerModified(text, printerUri, printerName, printerState, printerStateReason, acceptingJobs);
    }

    void mockPrinterDeleted(
        const QString &text,
        const QString &printerUri,
        const QString &printerName,
        uint printerState,
        const QString &printerStateReason,
        bool acceptingJobs
    )
    {
        Q_EMIT printerDeleted(text, printerUri, printerName, printerState, printerStateReason, acceptingJobs);
    }

    void mockJobCreated(
        const QString &text, const QString &printer_uri,
        const QString &printer_name, uint printer_state,
        const QString &printer_state_reasons, bool printer_is_accepting_jobs,
        uint job_id, uint job_state, const QString &job_state_reasons,
        const QString &job_name, uint job_impressions_completed
    )
    {
        Q_EMIT jobCreated(
            text, printer_uri, printer_name, printer_state,
            printer_state_reasons, printer_is_accepting_jobs, job_id,
            job_state, job_state_reasons, job_name, job_impressions_completed
        );
    }

    void mockDriversLoaded(const QList<PrinterDriver> &drivers)
    {
        Q_EMIT printerDriversLoaded(drivers);
    }

    void mockDriversLoaded(const QString &errorMessage)
    {
        Q_EMIT printerDriversFailedToLoad(errorMessage);
    }

    void mockPrinterLoaded(QSharedPointer<Printer> printer)
    {
        Q_EMIT printerLoaded(printer);
    }

    QString returnValue = QString::null;

    // Map from printer to key/val.
    QMap<QString, QMap<QString, QVariant>> printerOptions;

    QMap<QString, bool> enableds;
    QMap<QString, bool> shareds;
    QMap<QString, QString> infos;
    QMap<QString, QString> locations;
    QMap<QString, PrinterEnum::ErrorPolicy> errorPolicies;
    QMap<QString, PrinterEnum::OperationPolicy> operationPolicies;

    QString m_description = QString::null;
    QString m_location = QString::null;
    QString m_makeAndModel = QString::null;
    QString m_defaultPrinterName = QString::null;

    PrinterEnum::State m_state = PrinterEnum::State::IdleState;

    QPageSize m_defaultPageSize;
    QList<QPageSize> m_supportedPageSizes;

    QList<PrinterEnum::DuplexMode> m_supportedDuplexModes;

    QStringList m_availablePrinterNames;
    QList<QSharedPointer<Printer>> m_availablePrinters;
    QList<QSharedPointer<PrinterJob>> m_jobs;

public Q_SLOTS:
    virtual void refresh() override
    {
    }
};

#endif // USC_PRINTERS_MOCK_BACKEND_H
