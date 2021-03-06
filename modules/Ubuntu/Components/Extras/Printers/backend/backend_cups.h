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

#ifndef USC_PRINTERS_CUPS_BACKEND_H
#define USC_PRINTERS_CUPS_BACKEND_H

#include "backend/backend.h"
#include "cups/ippclient.h"
#include "cupsdnotifier.h" // Note: this file was generated.

#include <cups/cups.h>

#include <QPrinterInfo>
#include <QSet>

class PRINTERS_DECL_EXPORT PrinterCupsBackend : public PrinterBackend
{
    Q_OBJECT
public:
    explicit PrinterCupsBackend(IppClient *client, QPrinterInfo info,
                                OrgCupsCupsdNotifierInterface* notifier,
                                QObject *parent = Q_NULLPTR);
    virtual ~PrinterCupsBackend() override;

    virtual bool holdsDefinition() const override;

    virtual QString printerAdd(const QString &name,
                               const QString &uri,
                               const QString &ppdFile,
                               const QString &info,
                               const QString &location) override;
    virtual QString printerAddWithPpd(const QString &name,
                                      const QString &uri,
                                      const QString &ppdFileName,
                                      const QString &info,
                                      const QString &location) override;
    virtual QString printerDelete(const QString &name) override;
    virtual QString printerSetDefault(const QString &name) override;
    virtual QString printerSetEnabled(const QString &name,
                                      const bool enabled) override;
    virtual QString printerSetAcceptJobs(
        const QString &name,
        const bool accept,
        const QString &reason = QString::null) override;
    virtual QString printerSetCopies(
        const QString &name, const int &copies) override;
    virtual QString printerSetShared(const QString &name,
                                     const bool shared) override;
    virtual QString printerSetInfo(const QString &name,
                                   const QString &info) override;
    virtual QString printerAddOption(const QString &name,
                                     const QString &option,
                                     const QStringList &values) override;

    virtual QVariant printerGetOption(const QString &name,
                                      const QString &option) const override;
    virtual QMap<QString, QVariant> printerGetOptions(
        const QString &name, const QStringList &options
    ) const override;
    // FIXME: maybe have a PrinterDest iface that has a CupsDest impl?
    virtual cups_dest_t* makeDest(const QString &name,
                                  const PrinterJob *options) override;

    virtual void cancelJob(const QString &name, const int jobId) override;
    virtual void holdJob(const QString &name, const int jobId) override;
    virtual void releaseJob(const QString &name, const int jobId) override;
    virtual int printFileToDest(const QString &filepath,
                                const QString &title,
                                const cups_dest_t *dest) override;
    virtual QList<QSharedPointer<PrinterJob>> printerGetJobs() override;
    virtual QSharedPointer<PrinterJob> printerGetJob(
            const QString &printerName, const int jobId) override;

    virtual QString printerName() const override;
    virtual QString description() const override;
    virtual QString location() const override;
    virtual QString makeAndModel() const override;
    virtual bool isRemote() const override;

    virtual PrinterEnum::State state() const override;
    virtual QList<QPageSize> supportedPageSizes() const override;
    virtual QPageSize defaultPageSize() const override;
    virtual bool supportsCustomPageSizes() const override;

    virtual QPageSize minimumPhysicalPageSize() const override;
    virtual QPageSize maximumPhysicalPageSize() const override;
    virtual QList<int> supportedResolutions() const override;
    virtual PrinterEnum::DuplexMode defaultDuplexMode() const override;
    virtual QList<PrinterEnum::DuplexMode> supportedDuplexModes() const override;

    virtual QList<QSharedPointer<Printer>> availablePrinters() override;
    virtual QStringList availablePrinterNames() override;
    virtual QSharedPointer<Printer> getPrinter(const QString &printerName) override;
    virtual QString defaultPrinterName() override;

    virtual void requestJobExtendedAttributes(
            QSharedPointer<Printer> printer,
            QSharedPointer<PrinterJob> job) override;
    virtual void requestPrinterDrivers() override;
    virtual void requestPrinter(const QString &printerName) override;
    virtual QMap<QString, QVariant> printerGetJobAttributes(
        const QString &name, const int jobId) override;

public Q_SLOTS:
    virtual void refresh() override;
    void createSubscription();

    /* Starts a search for devices. Devices are synonymous with network devices
    that may potentially be queues. This search will take at minimum 5 seconds,
    normally longer. */
    void searchForDevices();

Q_SIGNALS:
    void cancelWorkers();
    void printerDriversLoaded(const QList<PrinterDriver> &drivers);
    void printerDriversFailedToLoad(const QString &errorMessage);
    void requestPrinterDriverCancel();

private:
    void cancelSubscription();
    void cancelPrinterDriverRequest();
    QList<cups_job_t *> getCupsJobs(const QString &name = QStringLiteral());

    QString getPrinterName(const QString &name) const;
    QString getPrinterInstance(const QString &name) const;
    cups_dest_t* getDest(const QString &name) const;
    ppd_file_t* getPpd(const QString &name) const;
    bool isExtendedAttribute(const QString &attributeName) const;

    const QStringList m_knownQualityOptions;
    const QStringList m_extendedAttributeNames;
    IppClient *m_client;
    QPrinterInfo m_info;
    OrgCupsCupsdNotifierInterface *m_notifier;
    int m_cupsSubscriptionId;
    mutable QMap<QString, cups_dest_t*> m_dests; // Printer name, dest.
    mutable QMap<QString, ppd_file_t*> m_ppds; // Printer name, ppd.
    QSet<QString> m_activePrinterRequests;
    QSet<QPair<QString, int>> m_activeJobRequests;

private Q_SLOTS:
    void onJobLoaded(QString printerName, int jobId,
                     QMap<QString, QVariant> attributes);
    void onPrinterLoaded(QSharedPointer<Printer> printer);
};

#endif // USC_PRINTERS_CUPS_BACKEND_H
