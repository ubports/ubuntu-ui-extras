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

#include "plugin.h"

#include "enums.h"
#include "i18n.h"
#include "structs.h"

// #include "models/printermodel.h"
#include "printer/printer.h"
#include "printer/printerjob.h"
#include "printers/printers.h"

#include <QtQml/qqml.h>
#include <QList>

#define I18N_DOMAIN "ubuntu-settings-components"

static QObject* p_singletonprovider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new Printers;
}

void UbuntuSettingsComponentsPrintersPlugin::registerTypes(const char *uri)
{
    initTr(I18N_DOMAIN, NULL);

    qmlRegisterSingletonType<Printers>(
        uri, 0, 1, "Printers", p_singletonprovider
    );
    qmlRegisterUncreatableType<Printer>(
        uri, 0, 1, "Printer", "use Printers to get a list of Printers."
    );

    qmlRegisterUncreatableType<PrinterJob>(uri, 0, 1, "PrinterJob",
                                           "use Printers to create jobs.");

    qmlRegisterUncreatableType<PrinterEnum>(uri, 0, 1, "PrinterEnum", "Is an enum");
    qRegisterMetaType<QList<PrinterDriver>>("QList<PrinterDriver>");
    qRegisterMetaType<QList<QSharedPointer<Printer>>>("QList<QSharedPointer<Printer>>");
}
