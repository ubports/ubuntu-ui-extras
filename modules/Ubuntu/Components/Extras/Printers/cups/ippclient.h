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

#ifndef USC_PRINTERS_CUPS_IPPCLIENT_H
#define USC_PRINTERS_CUPS_IPPCLIENT_H

#include "structs.h"

#include <cups/cups.h>
#include <cups/http.h>
#include <cups/ipp.h>
#include <cups/ppd.h>

#include <QString>
#include <QStringList>

/* From https://bugzilla.novell.com/show_bug.cgi?id=447444#c5
 * We need to define a maximum length for strings to avoid cups
 * thinking there are multiple lines.
 */
#define CPH_STR_MAXLEN 512

class IppClient
{
public:
    explicit IppClient();
    ~IppClient();

    bool printerDelete(const QString &printerName);
    bool printerAdd(const QString &printerName,
                    const QString &printerUri,
                    const QString &ppdFile,
                    const QString &info,
                    const QString &location);

    bool printerAddWithPpdFile(const QString &printerName,
                               const QString &printerUri,
                               const QString &ppdFileName,
                               const QString &info,
                               const QString &location);
    bool printerSetDefault(const QString &printerName);
    bool printerSetEnabled(const QString &printerName, const bool enabled);
    bool printerSetAcceptJobs(const QString &printerName, const bool accept,
                              const QString &reason);
    bool printerClassSetInfo(const QString &name, const QString &info);
    bool printerClassSetOption(const QString &name, const QString &option,
                               const QStringList &values);
    ppd_file_t* getPpdFile(const QString &name, const QString &instance) const;
    cups_dest_t* getDest(const QString &name, const QString &instance) const;

    QMap<QString, QVariant> printerGetJobAttributes(const int jobId);

    QString getLastError() const;

    // Note: This response needs to be free by the caller.
    ipp_t* createPrinterDriversRequest(
        const QString &deviceId, const QString &language,
        const QString &makeModel, const QString &product,
        const QStringList &includeSchemes, const QStringList &excludeSchemes
    );
    int createSubscription();
    void cancelSubscription(const int &subscriptionId);

private:
    enum CupsResource
    {
        CupsResourceRoot = 0,
        CupsResourceAdmin,
        CupsResourceJobs,
    };

    bool sendNewPrinterClassRequest(const QString &printerName,
                                    ipp_tag_t group,
                                    ipp_tag_t type,
                                    const QString &name,
                                    const QString &value);
    static void addPrinterUri(ipp_t *request, const QString &name);
    static void addRequestingUsername(ipp_t *request, const QString &username);
    static const QString getResource(const CupsResource &resource);
    static bool isPrinterNameValid(const QString &name);
    static void addClassUri(ipp_t *request, const QString &name);
    static bool isStringValid(const QString &string,
                              const bool checkNull = false,
                              const int maxLength = 512);
    static bool isStringPrintable(const QString &string, const bool checkNull,
                                  const int maxLength);
    QString preparePpdForOptions(const QString &ppdfile,
                                 cups_option_t *options,
                                 int numOptions);
    bool printerIsClass(const QString &name);
    void setInternalStatus(const QString &status);
    bool postRequest(ipp_t *request, const QString &file,
                     const CupsResource &resource);
    bool sendRequest(ipp_t *request, const CupsResource &resource);
    bool sendNewSimpleRequest(ipp_op_t op, const QString &printerName,
                              const CupsResource &resource);
    bool handleReply(ipp_t *reply);
    bool isReplyOk(ipp_t *reply, bool deleteIfReplyNotOk);
    void setErrorFromReply(ipp_t *reply);
    QVariant getAttributeValue(ipp_attribute_t *attr, int index=-1) const;

    http_t *m_connection;
    ipp_status_t m_lastStatus = IPP_OK;
    mutable QString m_internalStatus = QString::null;
};


#endif // USC_PRINTERS_CUPS_IPPCLIENT_H
