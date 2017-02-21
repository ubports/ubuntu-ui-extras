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

#include "printerdriverloader.h"

PrinterDriverLoader::PrinterDriverLoader(
        const QString &deviceId, const QString &language,
        const QString &makeModel, const QString &product,
        const QStringList &includeSchemes, const QStringList &excludeSchemes)
    : m_deviceId(deviceId)
    , m_language(language)
    , m_makeModel(makeModel)
    , m_product(product)
    , m_includeSchemes(includeSchemes)
    , m_excludeSchemes(excludeSchemes)
{
}

PrinterDriverLoader::~PrinterDriverLoader()
{
}

void PrinterDriverLoader::process()
{
    m_running = true;

    ipp_t* response = client.createPrinterDriversRequest(
        m_deviceId, m_language, m_makeModel, m_product, m_includeSchemes,
        m_excludeSchemes
    );

    // Note: if the response somehow fails, we return.
    if (!response || ippGetStatusCode(response) > IPP_OK_CONFLICT) {
        QString err(cupsLastErrorString());
        qWarning() << Q_FUNC_INFO << "Cups HTTP error:" << err;

        if (response)
            ippDelete(response);

        Q_EMIT error(err);
        Q_EMIT finished();
        return;
    }

    ipp_attribute_t *attr;
    QByteArray ppdDeviceId;
    QByteArray ppdLanguage;
    QByteArray ppdMakeModel;
    QByteArray ppdName;

    // cups_option_t option;
    QList<PrinterDriver> drivers;

    for (attr = ippFirstAttribute(response); attr != NULL && m_running; attr = ippNextAttribute(response)) {

        while (attr != NULL && ippGetGroupTag(attr) != IPP_TAG_PRINTER)
            attr = ippNextAttribute(response);

        if (attr == NULL)
            break;

        // Pull the needed attributes from this PPD...
        ppdDeviceId = "NONE";
        ppdLanguage.clear();
        ppdMakeModel.clear();
        ppdName.clear();

        while (attr != NULL && ippGetGroupTag(attr) == IPP_TAG_PRINTER) {
            if (!strcmp(ippGetName(attr), "ppd-device-id") &&
                 ippGetValueTag(attr) == IPP_TAG_TEXT) {
                ppdDeviceId = ippGetString(attr, 0, NULL);
            } else if (!strcmp(ippGetName(attr), "ppd-natural-language") &&
                       ippGetValueTag(attr) == IPP_TAG_LANGUAGE) {
                ppdLanguage = ippGetString(attr, 0, NULL);

            } else if (!strcmp(ippGetName(attr), "ppd-make-and-model") &&
                       ippGetValueTag(attr) == IPP_TAG_TEXT) {
                ppdMakeModel = ippGetString(attr, 0, NULL);
            } else if (!strcmp(ippGetName(attr), "ppd-name") &&
                       ippGetValueTag(attr) == IPP_TAG_NAME) {

                ppdName = ippGetString(attr, 0, NULL);
            }

            attr = ippNextAttribute(response);
        }

        // See if we have everything needed...
        if (ppdLanguage.isEmpty() || ppdMakeModel.isEmpty() ||
            ppdName.isEmpty()) {
            if (attr == NULL)
                break;
            else
                continue;
        }

        PrinterDriver m;
        m.name = ppdName;
        m.deviceId = ppdDeviceId;
        m.makeModel = ppdMakeModel;
        m.language = ppdLanguage;

        drivers.append(m);
    }

    ippDelete(response);

    Q_EMIT loaded(drivers);
    Q_EMIT finished();
}

void PrinterDriverLoader::cancel()
{
    m_running = false;
}
