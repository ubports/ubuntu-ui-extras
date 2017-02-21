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

#ifndef USC_PRINTERS_STRUCTS_H
#define USC_PRINTERS_STRUCTS_H

#include "enums.h"
#include "i18n.h"

#include <QtCore/QMap>
#include <QDebug>
#include <QMetaType>

struct InkLevels
{
public:
    QMap<PrinterEnum::CartridgeType, uint> levels;
};

struct ColorModel
{
public:
    QString name = QString::null; // Gray, RGB, CMYK or anything [1], really.
    QString text = QString::null;
    PrinterEnum::ColorModelType colorType
        = PrinterEnum::ColorModelType::UnknownType;

    QString originalOption = QString::null;

    bool operator==(const ColorModel& a) const
    {
        return (name == a.name && originalOption == a.originalOption);
    }
    bool operator!=(const ColorModel& a) const
    {
        return !(*this == a);
    }
    void operator=(const ColorModel &m) {
        name = m.name;
        text = m.text;
        originalOption = m.originalOption;
    }
};

// [1] https://www.cups.org/doc/ppd-compiler.html#COLOR

struct PrintQuality
{
public:
    QString name = QString::null;
    QString text = QString::null;

    // PrintQuality, StpQuality, HPPrintQuality, etc.
    QString originalOption = QString::null;

    bool operator==(const PrintQuality& a) const
    {
        return (name == a.name && originalOption == a.originalOption);
    }
    bool operator !=(const PrintQuality& a) const
    {
        return !(operator ==(a));
    }
};

struct PrinterDriver
{
public:
    QByteArray name;
    QByteArray deviceId;
    QByteArray language;
    QByteArray makeModel;

    QString toString() const {
        return QString("%1 [%2]").arg(QString::fromUtf8(makeModel))
                                 .arg(QString::fromUtf8(language));
    }
};

Q_DECLARE_TYPEINFO(ColorModel, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(ColorModel)

Q_DECLARE_TYPEINFO(PrintQuality, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(PrintQuality)

Q_DECLARE_TYPEINFO(PrinterDriver, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(PrinterDriver)
Q_DECLARE_METATYPE(QList<PrinterDriver>)

#endif // USC_PRINTERS_STRUCTS_H
