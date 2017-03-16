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
        return m_holdsDefinition;
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
        m_defaultPrinterName = name;
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

    virtual QString printerSetCopies(const QString &name, const int &copies)
    {
        printerOptions[name].insert("Copies", copies);
        return returnValue;
    }

    virtual QString printerSetShared(const QString &name,
                                     const bool shared) override
    {
        printerOptions[name].insert("Shared", shared);
        return returnValue;
    }

    virtual QString printerSetInfo(const QString &name,
                                   const QString &info) override
    {
        infos.insert(name, info);
        return returnValue;
    }

    virtual QString printerAddOption(const QString &name,
                                     const QString &option,
                                     const QStringList &values) override
    {
        printerOptions[name].insert(option, values);
        return returnValue;
    }

    virtual QVariant printerGetOption(const QString &name,
                                      const QString &option) const override
    {
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

    virtual cups_dest_t* makeDest(const QString &name,
                                  const PrinterJob *options) override
    {
        Q_UNUSED(name);
        Q_UNUSED(options);
        return Q_NULLPTR;
    }

    virtual void cancelJob(const QString &name, const int jobId) override
    {
        QList<int> toRemove;

        for (int i=0; i < m_jobs.count(); i++) {
            QSharedPointer<PrinterJob> job = m_jobs[i];

            if (job->printerName() == name && job->jobId() == jobId) {
                toRemove << i;
            }
        }

        for (int i=0; i < toRemove.count(); i++) {
            QSharedPointer<PrinterJob> job = m_jobs.takeAt(toRemove[i] - i);

            Q_EMIT jobCompleted(job->title(), "", job->printerName(), 1, "", true, job->jobId(), 1, "", "", 1);
        }
    }

    virtual void holdJob(const QString &printerName, const int jobId) override
    {
        // Change the faked job state
        Q_FOREACH(auto job, m_jobs) {
            if (job->printerName() == printerName
                    && job->jobId() == jobId) {
                job->setState(PrinterEnum::JobState::Held);
            }
        }

        Q_EMIT jobState("", "", printerName, 1, "", true, jobId, static_cast<uint>(PrinterEnum::JobState::Held), "", "", 1);
    }

    virtual void releaseJob(const QString &printerName, const int jobId) override
    {
        // Change the faked job state
        Q_FOREACH(auto job, m_jobs) {
            if (job->printerName() == printerName
                    && job->jobId() == jobId) {
                job->setState(PrinterEnum::JobState::Pending);
            }
        }

        Q_EMIT jobState("", "", printerName, 1, "", true, jobId, static_cast<uint>(PrinterEnum::JobState::Pending), "", "", 1);
    }

    virtual int printFileToDest(const QString &filepath,
                                const QString &title,
                                const cups_dest_t *dest) override
    {
        Q_UNUSED(filepath);
        Q_UNUSED(title);
        Q_UNUSED(dest);

        Q_EMIT printToFile(filepath, title);

        return -1;
    }

    virtual QList<QSharedPointer<PrinterJob>> printerGetJobs() override
    {
        return m_jobs;
    }

    virtual QMap<QString, QVariant> printerGetJobAttributes(
            const QString &name, const int jobId) override
    {
        QMap<QString, QVariant> attributes;

        Q_FOREACH(auto job, m_jobs) {
            if (job->printerName() == name
                    && job->jobId() == jobId) {
                // Emulate reverse of PrinterJob::loadAttributes
                // using local jobs defined in tests
                attributes.insert("Collate", job->collate());
                attributes.insert("copies", job->copies());
                attributes.insert("ColorModel", job->getColorModel().name);
                attributes.insert("CompletedTime", job->completedTime());
                attributes.insert("CreationTime", job->creationTime());
                attributes.insert("Duplex", Utils::duplexModeToPpdChoice(job->getDuplexMode()));
                attributes.insert("landscape", job->landscape());
                attributes.insert("messages", job->messages());
                if (job->printRangeMode() == PrinterEnum::PrintRange::AllPages) {
                    attributes.insert("page-ranges", QStringList());
                } else {
                    attributes.insert("page-ranges", job->printRange().split(QLocale::system().groupSeparator()));
                }
                attributes.insert("ProcessingTime", job->processingTime());
                attributes.insert("Quality", job->getPrintQuality().name);
                attributes.insert("OutputOrder", job->reverse() ? "Reverse" : "Normal");
                attributes.insert("Size", job->size());
                attributes.insert("User", job->user());

                break;
            }
        }

        return attributes;
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

    virtual bool isRemote() const override
    {
        return m_remote;
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


    virtual void requestPrinter(const QString &printerName) override
    {
        m_requestedPrinters << printerName;
    }

    virtual void requestJobExtendedAttributes(QSharedPointer<Printer> printer, QSharedPointer<PrinterJob> job) override
    {
        QMap<QString, QVariant> attributes = printerGetJobAttributes(printer->name(), job->jobId());

        Q_EMIT jobLoaded(printer->name(), job->jobId(), attributes);
    }

    virtual PrinterEnum::PrinterType type() const override
    {
        return m_type;
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

    void mockJobCompleted(
        const QString &text, const QString &printer_uri,
        const QString &printer_name, uint printer_state,
        const QString &printer_state_reasons, bool printer_is_accepting_jobs,
        uint job_id, uint job_state, const QString &job_state_reasons,
        const QString &job_name, uint job_impressions_completed)
    {
        Q_EMIT jobCompleted(
            text, printer_uri, printer_name, printer_state,
            printer_state_reasons, printer_is_accepting_jobs, job_id,
            job_state, job_state_reasons, job_name, job_impressions_completed
        );
    }

    void mockJobState(
        const QString &text, const QString &printer_uri,
        const QString &printer_name, uint printer_state,
        const QString &printer_state_reasons, bool printer_is_accepting_jobs,
        uint job_id, uint job_state, const QString &job_state_reasons,
        const QString &job_name, uint job_impressions_completed)
    {
        Q_EMIT jobState(
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

    void mockDeviceFound(const Device &device)
    {
        Q_EMIT deviceFound(device);
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

    bool m_holdsDefinition = true;
    bool m_remote = false;

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
    QStringList m_requestedPrinters;

    PrinterEnum::PrinterType m_type = PrinterEnum::PrinterType::ProxyType;

Q_SIGNALS:
    void printToFile(const QString &filepath, const QString &title);
};

#endif // USC_PRINTERS_MOCK_BACKEND_H
