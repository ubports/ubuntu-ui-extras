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

struct Device
{
public:
    QString cls;
    QString id;
    QString info;
    QString makeModel;
    QString uri;
    QString location;

    QString toString() const {
        return QString("Class: %1, Id: %2, Info: %3, MakeModel: %4, Uri: %5, Location: %6")
            .arg(cls)
            .arg(id)
            .arg(info)
            .arg(makeModel)
            .arg(uri)
            .arg(location);
    }

    bool operator==(const Device &other)
    {
        return ((cls == other.cls) && (id == other.id) && (info == other.info) &&
                (makeModel == other.makeModel) && (uri == other.uri) &&
                (location == other.location));
    }
};

Q_DECLARE_TYPEINFO(ColorModel, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(ColorModel)

Q_DECLARE_TYPEINFO(PrintQuality, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(PrintQuality)

Q_DECLARE_TYPEINFO(PrinterDriver, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(PrinterDriver)
Q_DECLARE_METATYPE(QList<PrinterDriver>)

Q_DECLARE_TYPEINFO(Device, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(Device)

#endif // USC_PRINTERS_STRUCTS_H
