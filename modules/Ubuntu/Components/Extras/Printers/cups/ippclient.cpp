/*
 * Copyright (C) 2017 Canonical, Ltd.
 * Copyright (C) 2014 John Layt <jlayt@kde.org>
 * Copyright (C) 2002, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013, 2014 Red Hat, Inc.
 * Copyright (C) 2008 Novell, Inc.
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

#include "cups/ippclient.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <QDebug>
#include <QDateTime>
#include <QTimeZone>
#include <QUrl>

IppClient::IppClient()
    : m_connection(httpConnectEncrypt(cupsServer(),
                                      ippPort(),
                                      cupsEncryption()))
{
    if (!m_connection) {
        qCritical("Failed to connect to cupsd");
    } else {
        qDebug("Successfully connected to cupsd.");
    }
}

IppClient::~IppClient()
{
    if (m_connection)
        httpClose(m_connection);
}

bool IppClient::printerDelete(const QString &printerName)
{
    return sendNewSimpleRequest(CUPS_DELETE_PRINTER, printerName.toUtf8(),
                                CupsResource::CupsResourceAdmin);
}

bool IppClient::printerAdd(const QString &printerName,
                           const QString &printerUri,
                           const QString &ppdFile,
                           const QString &info,
                           const QString &location)
{
    ipp_t *request;

    if (!isPrinterNameValid(printerName)) {
        setInternalStatus(QString("%1 is not a valid printer name.").arg(printerName));
        return false;
    }

    if (!isStringValid(info)) {
        setInternalStatus(QString("%1 is not a valid description.").arg(info));
        return false;
    }

    if (!isStringValid(location)) {
        setInternalStatus(QString("%1 is not a valid location.").arg(location));
        return false;
    }

    if (!isStringValid(ppdFile)) {
        setInternalStatus(QString("%1 is not a valid ppd file.").arg(ppdFile));
        return false;
    }

    if (!isStringValid(printerUri)) {
        setInternalStatus(QString("%1 is not a valid printer uri.").arg(printerUri));
        return false;
    }


    request = ippNewRequest (CUPS_ADD_MODIFY_PRINTER);
    addPrinterUri(request, printerName);
    addRequestingUsername(request, NULL);

    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                 "printer-name", NULL, printerName.toUtf8());

    if (!ppdFile.isEmpty()) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                     "ppd-name", NULL, ppdFile.toUtf8());
    }
    if (!printerUri.isEmpty()) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                     "device-uri", NULL, printerUri.toUtf8());
    }
    if (!info.isEmpty()) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                     "printer-info", NULL, info.toUtf8());
    }
    if (!location.isEmpty()) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                     "printer-location", NULL, location.toUtf8());
    }

    return sendRequest(request, CupsResourceAdmin);
}

bool IppClient::printerAddWithPpdFile(const QString &printerName,
                                      const QString &printerUri,
                                      const QString &ppdFileName,
                                      const QString &info,
                                      const QString &location)
{
    ipp_t *request;

    if (!isPrinterNameValid(printerName)) {
        setInternalStatus(QString("%1 is not a valid printer name.").arg(printerName));
        return false;
    }

    if (!isStringValid(info)) {
        setInternalStatus(QString("%1 is not a valid description.").arg(info));
        return false;
    }

    if (!isStringValid(location)) {
        setInternalStatus(QString("%1 is not a valid location.").arg(location));
        return false;
    }

    if (!isStringValid(ppdFileName)) {
        setInternalStatus(QString("%1 is not a valid ppd file name.").arg(ppdFileName));
        return false;
    }

    if (!isStringValid(printerUri)) {
        setInternalStatus(QString("%1 is not a valid printer uri.").arg(printerUri));
        return false;
    }

    request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
    addPrinterUri(request, printerName);
    addRequestingUsername(request, NULL);

    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                 "printer-name", NULL, printerName.toUtf8());

    /* In this specific case of ADD_MODIFY, the URI can be NULL/empty since
     * we provide a complete PPD. And cups fails if we pass an empty
     * string. */
    if (!printerUri.isEmpty()) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                     "device-uri", NULL, printerUri.toUtf8());
    }

    if (!info.isEmpty()) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                     "printer-info", NULL, info.toUtf8());
    }
    if (!location.isEmpty()) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                     "printer-location", NULL, location.toUtf8());
    }

    return postRequest(request, ppdFileName.toUtf8(), CupsResourceAdmin);
}

bool IppClient::printerSetDefault(const QString &printerName)
{
    return sendNewSimpleRequest(CUPS_SET_DEFAULT, printerName.toUtf8(),
                                CupsResource::CupsResourceAdmin);
}

bool IppClient::printerSetEnabled(const QString &printerName,
                                  const bool enabled)
{
    ipp_op_t op;
    op = enabled ? IPP_RESUME_PRINTER : IPP_PAUSE_PRINTER;
    return sendNewSimpleRequest(op, printerName, CupsResourceAdmin);
}

/* reason must be empty if accept is true */
bool IppClient::printerSetAcceptJobs(const QString &printerName,
                                     const bool accept,
                                     const QString &reason)
{
    ipp_t *request;

    if (accept && !reason.isEmpty()) {
        setInternalStatus("Accepting jobs does not take a reason.");
        return false;
    }

    if (!isPrinterNameValid(printerName)) {
        setInternalStatus(QString("%1 is not a valid printer name.").arg(printerName));
        return false;
    }

    if (!isStringValid(reason)) {
        setInternalStatus(QString("%1 is not a valid reason.").arg(reason));
        return false;
    }

    if (accept) {
        return sendNewSimpleRequest(CUPS_ACCEPT_JOBS, printerName.toUtf8(),
                                    CupsResourceAdmin);
    } else {
        request = ippNewRequest(CUPS_REJECT_JOBS);
        addPrinterUri(request, printerName);
        addRequestingUsername(request, NULL);

        if (!reason.isEmpty())
            ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT,
                         "printer-state-message", NULL, reason.toUtf8());

        return sendRequest(request, CupsResourceAdmin);
    }
}


bool IppClient::printerClassSetInfo(const QString &name,
                                       const QString &info)
{
    if (!isPrinterNameValid(name)) {
        setInternalStatus(QString("%1 is not a valid printer name.").arg(name));
        return false;
    }

    if (!isStringValid(info)) {
        setInternalStatus(QString("%1 is not a valid description.").arg(info));
        return false;
    }

    return sendNewPrinterClassRequest(name, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                                      "printer-info", info);
}

bool IppClient::printerClassSetOption(const QString &name,
                                      const QString &option,
                                      const QStringList &values)
{
    bool isClass;
    int length = 0;
    ipp_t *request;
    ipp_attribute_t *attr;
    QString newPpdFile;
    bool retval;

    if (!isPrinterNameValid(name)) {
        setInternalStatus(QString("%1 is not a valid printer name.").arg(name));
        return false;
    }

    if (!isStringValid(option)) {
        setInternalStatus(QString("%1 is not a valid option.").arg(option));
        return false;
    }

    Q_FOREACH(const QString &val, values) {
        if (!isStringValid(val)) {
            setInternalStatus(QString("%1 is not a valid value.").arg(val));
            return false;
        }
        length++;
    }

    if (length == 0) {
        setInternalStatus("No valid values.");
        return false;
    }

    isClass = printerIsClass(name);

    /* We permit only one value to change in PPD file because we are setting
     * default value in it. */
    if (!isClass && length == 1) {
        cups_option_t *options = NULL;
        int numOptions = 0;
        QString ppdfile;

        numOptions = cupsAddOption(option.toUtf8(),
                                   values[0].toUtf8(),
                                   numOptions, &options);

        ppdfile = QString(cupsGetPPD(name.toUtf8()));

        newPpdFile = preparePpdForOptions(ppdfile.toUtf8(),
                                          options, numOptions).toLatin1().data();

        unlink(ppdfile.toUtf8());
        cupsFreeOptions(numOptions, options);
    }

    if (isClass) {
        request = ippNewRequest(CUPS_ADD_MODIFY_CLASS);
        addClassUri(request, name);
    } else {
        request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
        addPrinterUri(request, name);
    }

    addRequestingUsername(request, NULL);

    if (length == 1) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                     option.toUtf8(),
                     NULL,
                     values[0].toUtf8());
    } else {
        int i;

        attr = ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                             option.toUtf8(), length, NULL, NULL);

        for (i = 0; i < length; i++)
            ippSetString(request, &attr, i, values[i].toUtf8());
    }

    if (!newPpdFile.isEmpty()) {
        retval = postRequest(request, newPpdFile, CupsResourceAdmin);

        unlink(newPpdFile.toUtf8());
        // TODO: fix leak here.
    } else {
        retval = sendRequest(request, CupsResourceAdmin);
    }

    return retval;
}

QMap<QString, QVariant> IppClient::printerGetJobAttributes(const int jobId)
{
    ipp_t *request;
    QMap<QString, QVariant> map;

    // Construct request
    request = ippNewRequest(IPP_GET_JOB_ATTRIBUTES);
    QString uri = QStringLiteral("ipp://localhost/jobs/") + QString::number(jobId);
    qDebug() << "URI:" << uri;

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri", NULL, uri.toStdString().data());


    // Send request and construct reply
    ipp_t *reply;
    const QString resourceChar = getResource(CupsResourceRoot);
    reply = cupsDoRequest(m_connection, request,
                          resourceChar.toUtf8());

    // Check if the reply is OK
    if (isReplyOk(reply, false)) {
        // Loop through the attributes
        ipp_attribute_t *attr;

        for (attr = ippFirstAttribute(reply); attr; attr = ippNextAttribute(reply)) {
            QVariant value = getAttributeValue(attr);
            map.insert(ippGetName(attr), value);
        }
    } else {
        qWarning() << "Not able to get attributes of job:" << jobId;
    }

    // Destruct the reply if valid
    if (reply) {
        ippDelete(reply);
    }

    return map;
}


/* This function sets given options to specified values in file 'ppdfile'.
 * This needs to be done because of applications which use content of PPD files
 * instead of IPP attributes.
 * CUPS doesn't do this automatically (but hopefully will starting with 1.6) */
QString IppClient::preparePpdForOptions(const QString &ppdfile,
                                        cups_option_t *options, int numOptions)
{
    auto ppdfile_c = ppdfile.toUtf8();
    ppd_file_t *ppd;
    bool ppdchanged = false;
    QString result;
    QString error;
    char newppdfile[PATH_MAX];
    cups_file_t  *in = NULL;
    cups_file_t  *out = NULL;
    char line[CPH_STR_MAXLEN];
    char keyword[CPH_STR_MAXLEN];
    char *keyptr;
    ppd_choice_t *choice;
    QString value;
    QLatin1String defaultStr("*Default");

    ppd = ppdOpenFile(ppdfile_c);
    if (!ppd) {
        error = QString("Unable to open PPD file \"%1\": %2")
                .arg(ppdfile).arg(strerror(errno));
        setInternalStatus(error);
        goto out;
    }

    in = cupsFileOpen(ppdfile_c, "r");
    if (!in) {
        error = QString("Unable to open PPD file \"%1\": %2")
            .arg(ppdfile).arg(strerror(errno));
        setInternalStatus(error);
        goto out;
    }

    out = cupsTempFile2(newppdfile, sizeof(newppdfile));
    if (!out) {
        setInternalStatus("Unable to create temporary file");
        goto out;
    }

    /* Mark default values and values of options we are changing. */
    ppdMarkDefaults(ppd);
    cupsMarkOptions(ppd, numOptions, options);

    while (cupsFileGets(in, line, sizeof(line))) {
        QString line_qs(line);
        if (!line_qs.startsWith(defaultStr)) {
            cupsFilePrintf(out, "%s\n", line);
        } else {
            /* This part parses lines with *Default on their
             * beginning. For instance:
             *   "*DefaultResolution: 1200dpi" becomes:
             *     - keyword: Resolution
             *     - keyptr: 1200dpi
             */
            strncpy(keyword, line + defaultStr.size(), sizeof(keyword));

            for (keyptr = keyword; *keyptr; keyptr++)
                    if (*keyptr == ':' || isspace (*keyptr & 255))
                            break;

            *keyptr++ = '\0';
            while (isspace (*keyptr & 255))
                    keyptr++;

            QString keyword_sq(keyword);
            QString keyptr_qs(keyptr);

            /* We have to change PageSize if any of PageRegion,
             * PageSize, PaperDimension or ImageableArea changes.
             * We change PageRegion if PageSize is not available. */
            if (keyword_sq == "PageRegion" ||
                keyword_sq == "PageSize" ||
                keyword_sq == "PaperDimension" ||
                keyword_sq == "ImageableArea") {

                choice = ppdFindMarkedChoice(ppd, "PageSize");
                if (!choice)
                    choice = ppdFindMarkedChoice(ppd, "PageRegion");
            } else {
                choice = ppdFindMarkedChoice(ppd, keyword);
            }


            QString choice_qs;
            if (choice) {
                choice_qs = choice->choice;
            }

            if (choice && choice_qs != keyptr_qs) {
                /* We have to set the value in PPD manually if
                 * a custom value was passed in:
                 * cupsMarkOptions() marks the choice as
                 * "Custom". We want to set this value with our
                 * input. */
                if (choice_qs != "Custom") {
                    cupsFilePrintf(out,
                                   "*Default%s: %s\n",
                                   keyword,
                                   choice->choice);
                    ppdchanged = true;
                } else {
                    value = cupsGetOption(keyword, numOptions, options);
                    if (!value.isEmpty()) {
                        cupsFilePrintf(out,
                                       "*Default%s: %s\n",
                                       keyword,
                                       value.toStdString().c_str());
                        ppdchanged = true;
                    } else {
                        cupsFilePrintf(out, "%s\n", line);
                    }
                }
            } else {
                cupsFilePrintf(out, "%s\n", line);
            }
        }
    }

    if (ppdchanged)
        result = QString::fromUtf8(newppdfile);
    else
        unlink(newppdfile);

out:
    if (in)
        cupsFileClose(in);
    if (out)
        cupsFileClose(out);
    if (ppd)
        ppdClose(ppd);

    return result;
}


bool IppClient::sendNewPrinterClassRequest(const QString &printerName,
                                           ipp_tag_t group, ipp_tag_t type,
                                           const QString &name,
                                           const QString &value)
{
    ipp_t *request;

    request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
    addPrinterUri(request, printerName);
    addRequestingUsername(request, QString());
    ippAddString(request, group, type, name.toUtf8(), NULL,
                 value.toUtf8());

    if (sendRequest(request, CupsResource::CupsResourceAdmin))
        return true;

    // it failed, maybe it was a class?
    if (m_lastStatus != IPP_NOT_POSSIBLE) {
        return false;
    }

    // TODO: implement class modification <here>.
    return false;
}

void IppClient::addPrinterUri(ipp_t *request, const QString &name)
{
    QUrl uri(QString("ipp://localhost/printers/%1").arg(name));
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", NULL, uri.toEncoded().data());
}

void IppClient::addRequestingUsername(ipp_t *request, const QString &username)
{
    if (!username.isEmpty())
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                     "requesting-user-name", NULL,
                     username.toUtf8());
    else
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                     "requesting-user-name", NULL, cupsUser());
}

QString IppClient::getLastError() const
{
    return m_internalStatus;
}

const QString IppClient::getResource(const IppClient::CupsResource &resource)
{
    switch (resource) {
    case CupsResourceRoot:
        return "/";
    case CupsResourceAdmin:
        return "/admin/";
    case CupsResourceJobs:
        return "/jobs/";
    default:
        qCritical("Asking for a resource with no match.");
        return "/";
    }
}

bool IppClient::isPrinterNameValid(const QString &name)
{
    int i;
    int len;

    /* Quoting the lpadmin man page:
     *    CUPS allows printer names to contain any printable character
     *    except SPACE, TAB, "/", or  "#".
     * On top of that, validate_name() in lpadmin.c (from cups) checks that
     * the string is 127 characters long, or shorter. */

    /* no empty string */
    if (name.isEmpty())
        return false;

    len = name.size();
    /* no string that is too long; see comment at the beginning of the
     * validation code block */
    if (len > 127)
        return false;

    /* only printable characters, no space, no /, no # */
    for (i = 0; i < len; i++) {
        const QChar c = name.at(i);
        if (!c.isPrint())
            return false;
        if (c.isSpace())
            return false;
        if (c == '/' || c == '#')
            return false;
    }
    return true;
}

bool IppClient::isStringValid(const QString &string, const bool checkNull,
                              const int maxLength)
{
    if (isStringPrintable(string, checkNull, maxLength))
        return true;
    return false;
}

bool IppClient::isStringPrintable(const QString &string, const bool checkNull,
                                  const int maxLength)
{
    int i;
    int len;

    /* no null string */
    if (string.isNull())
        return !checkNull;

    len = string.size();
    if (maxLength > 0 && len > maxLength)
        return false;

    /* only printable characters */
    for (i = 0; i < len; i++) {
        const QChar c = string.at(i);
        if (!c.isPrint())
            return false;
    }
    return true;
}

void IppClient::setInternalStatus(const QString &status)
{
    if (!m_internalStatus.isNull()) {
        m_internalStatus = QString::null;
    }

    if (status.isNull()) {
        m_internalStatus = QString::null;
    } else {
        m_internalStatus = status;

        // Only used for errors for now.
        qCritical() << status;
    }
}

bool IppClient::postRequest(ipp_t *request, const QString &file,
                            const CupsResource &resource)
{
    ipp_t *reply;
    QString resourceChar;

    resourceChar = getResource(resource);

    if (!file.isEmpty())
        reply = cupsDoFileRequest(m_connection, request, resourceChar.toUtf8(),
                                  file.toUtf8());
    else
        reply = cupsDoFileRequest(m_connection, request, resourceChar.toUtf8(),
                                  NULL);

    return handleReply(reply);
}


bool IppClient::sendRequest(ipp_t *request, const CupsResource &resource)
{
    ipp_t *reply;
    const QString resourceChar = getResource(resource);
    reply = cupsDoRequest(m_connection, request,
                          resourceChar.toUtf8());
    return handleReply(reply);
}

bool IppClient::sendNewSimpleRequest(ipp_op_t op, const QString &printerName,
                                     const IppClient::CupsResource &resource)
{
    ipp_t *request;

    if (!isPrinterNameValid(printerName))
        return false;

    request = ippNewRequest(op);
    addPrinterUri(request, printerName);
    addRequestingUsername(request, NULL);

    return sendRequest(request, resource);
}

bool IppClient::handleReply(ipp_t *reply)
{
    bool retval;
    retval = isReplyOk(reply, false);
    if (reply)
        ippDelete(reply);

    return retval;
}

bool IppClient::isReplyOk(ipp_t *reply, bool deleteIfReplyNotOk)
{
    /* reset the internal status: we'll use the cups status */
    m_lastStatus = IPP_STATUS_CUPS_INVALID;

    if (reply && ippGetStatusCode(reply) <= IPP_OK_CONFLICT) {
        m_lastStatus = IPP_OK;
        return true;
    } else {
        setErrorFromReply(reply);
        qWarning() << Q_FUNC_INFO << "Cups HTTP error:" << cupsLastErrorString();

        if (deleteIfReplyNotOk && reply)
            ippDelete(reply);

        return false;
    }
}

void IppClient::setErrorFromReply(ipp_t *reply)
{
    if (reply)
        m_lastStatus = ippGetStatusCode(reply);
    else
        m_lastStatus = cupsLastError();
}

bool IppClient::printerIsClass(const QString &name)
{
    const char * const attrs[1] = { "member-names" };
    ipp_t *request;
    QString resource;
    ipp_t *reply;
    bool retval;

    // Class/Printer name validation is equal.
    if (!isPrinterNameValid(name)) {
        setInternalStatus(QString("%1 is not a valid printer name.").arg(name));
        return false;
    }

    request = ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES);
    addClassUri(request, name);
    addRequestingUsername(request, QString());
    ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                  "requested-attributes", 1, NULL, attrs);

    resource = getResource(CupsResource::CupsResourceRoot);
    reply = cupsDoRequest(m_connection, request, resource.toUtf8());

    if (!isReplyOk(reply, true))
        return true;

    /* Note: we need to look if the attribute is there, since we get a
     * reply if the name is a printer name and not a class name. The
     * attribute is the only way to distinguish the two cases. */
    retval = ippFindAttribute(reply, attrs[0], IPP_TAG_NAME) != NULL;

    if (reply)
        ippDelete(reply);

    return retval;
}

void IppClient::addClassUri(ipp_t *request, const QString &name)
{
    QUrl uri(QString("ipp://localhost/printers/%1").arg(name));
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", NULL, uri.toEncoded().data());
}

ppd_file_t* IppClient::getPpdFile(const QString &name,
                                  const QString &instance) const
{
    Q_UNUSED(instance);

    ppd_file_t* file = 0;
    const char *ppdFile = cupsGetPPD(name.toUtf8());
    if (ppdFile) {
        file = ppdOpenFile(ppdFile);
        unlink(ppdFile);
    }
    if (file) {
        ppdMarkDefaults(file);
    } else {
        file = 0;
    }

    return file;
}

cups_dest_t* IppClient::getDest(const QString &name,
                                const QString &instance) const
{
    cups_dest_t *dest = 0;

    if (instance.isEmpty()) {
        dest = cupsGetNamedDest(m_connection, name.toUtf8(), NULL);
    } else {
        dest = cupsGetNamedDest(m_connection, name.toUtf8(), instance.toUtf8());
    }
    return dest;
}

ipp_t* IppClient::createPrinterDriversRequest(
    const QString &deviceId, const QString &language, const QString &makeModel,
    const QString &product, const QStringList &includeSchemes,
    const QStringList &excludeSchemes
)
{
    Q_UNUSED(includeSchemes);
    Q_UNUSED(excludeSchemes);

    ipp_t *request;

    request = ippNewRequest(CUPS_GET_PPDS);

    if (!deviceId.isEmpty())
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT, "ppd-device-id",
                 NULL, deviceId.toUtf8());
    if (!language.isEmpty())
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_LANGUAGE, "ppd-language",
                 NULL, language.toUtf8());
    if (!makeModel.isEmpty())
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT, "ppd-make-and-model",
                 NULL, makeModel.toUtf8());
    if (!product.isEmpty())
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT, "ppd-product",
                 NULL, product.toUtf8());

    // Do the request and get return the response.
    const QString resourceChar = getResource(CupsResourceRoot);
    return cupsDoRequest(m_connection, request,
                         resourceChar.toUtf8());
}

int IppClient::createSubscription()
{
    ipp_t *req;
    ipp_t *resp;
    ipp_attribute_t *attr;
    int subscriptionId = -1;

    req = ippNewRequest(IPP_CREATE_PRINTER_SUBSCRIPTION);
    ippAddString(req, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", NULL, "/");
    ippAddString(req, IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD,
                 "notify-events", NULL, "all");
    ippAddString(req, IPP_TAG_SUBSCRIPTION, IPP_TAG_URI,
                 "notify-recipient-uri", NULL, "dbus://");
    ippAddInteger(req, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER,
                  "notify-lease-duration", 0);

    resp = cupsDoRequest(m_connection, req,
                         getResource(CupsResourceRoot).toUtf8());
    if (!isReplyOk(resp, true)) {
        return subscriptionId;
    }

    attr = ippFindAttribute(resp, "notify-subscription-id", IPP_TAG_INTEGER);

    if (!attr) {
        qWarning() << "ipp-create-printer-subscription response doesn't"
                       "  contain subscription id.";
    } else {
        subscriptionId = ippGetInteger(attr, 0);
    }

    ippDelete (resp);

    return subscriptionId;
}

void IppClient::cancelSubscription(const int &subscriptionId)
{
    ipp_t *req;
    ipp_t *resp;

    if (subscriptionId <= 0) {
        return;
    }

    req = ippNewRequest(IPP_CANCEL_SUBSCRIPTION);
    ippAddString(req, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", NULL, "/");
    ippAddInteger(req, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                  "notify-subscription-id", subscriptionId);

    resp = cupsDoRequest(m_connection, req,
                         getResource(CupsResourceRoot).toUtf8());
    if (!isReplyOk(resp, true)) {
        return;
    }

    ippDelete(resp);
}

QVariant IppClient::getAttributeValue(ipp_attribute_t *attr, int index) const
{
    QVariant var;

    if (ippGetCount(attr) > 1 && index < 0) {
        QList<QVariant> list;

        for (int i=0; i < ippGetCount(attr); i++) {
            list.append(getAttributeValue(attr, i));
        }

        var = QVariant::fromValue<QList<QVariant>>(list);
    } else {
        if (index == -1) {
            index = 0;
        }

        switch (ippGetValueTag(attr)) {
        case IPP_TAG_NAME:
        case IPP_TAG_TEXT:
        case IPP_TAG_KEYWORD:
        case IPP_TAG_URI:
        case IPP_TAG_CHARSET:
        case IPP_TAG_MIMETYPE:
        case IPP_TAG_LANGUAGE:
            var = QVariant::fromValue<QString>(ippGetString(attr, index, NULL));
            break;
        case IPP_TAG_INTEGER:
        case IPP_TAG_ENUM:
            var = QVariant::fromValue<int>(ippGetInteger(attr, index));
            break;
        case IPP_TAG_BOOLEAN:
            var = QVariant::fromValue<bool>(ippGetBoolean(attr, index));
            break;
        case IPP_TAG_RANGE: {
            QString range;
            int upper;
            int lower = ippGetRange(attr, index, &upper);

            // Build a string similar to "1-3" "5-" "8" "-4"
            if (lower != INT_MIN) {
                range += QString::number(lower);
            }

            if (lower != upper) {
                range += QStringLiteral("-");

                if (upper != INT_MAX) {
                    range += QString::number(upper);
                }
            }

            var = QVariant(range);
            break;
        }
        case IPP_TAG_NOVALUE:
            var = QVariant();
            break;
        case IPP_TAG_DATE: {
            time_t time = ippDateToTime(ippGetDate(attr, index));
            QDateTime datetime;
            datetime.setTimeZone(QTimeZone::systemTimeZone());
            datetime.setTime_t(time);

            var = QVariant::fromValue<QDateTime>(datetime);
            break;
        }
        default:
            qWarning() << "Unknown IPP value tab 0x" << ippGetValueTag(attr);
            break;
        }
    }

    return var;
}
