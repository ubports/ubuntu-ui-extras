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

#ifndef USC_PRINTERS_UTILS_H
#define USC_PRINTERS_UTILS_H

#include "enums.h"
#include "i18n.h"
#include "structs.h"

#include <cups/ppd.h>

#include <QString>
#include <QPrinter>

class Utils
{
public:
    static PrinterEnum::DuplexMode ppdChoiceToDuplexMode(const QString &choice)
    {
        if (choice == "DuplexTumble")
            return PrinterEnum::DuplexMode::DuplexShortSide;
        else if (choice == "DuplexNoTumble")
            return PrinterEnum::DuplexMode::DuplexLongSide;
        else
            return PrinterEnum::DuplexMode::DuplexNone;
    }

    static const QString duplexModeToPpdChoice(const PrinterEnum::DuplexMode &mode)
    {
        switch (mode) {
        case PrinterEnum::DuplexMode::DuplexShortSide:
            return "DuplexTumble";
        case PrinterEnum::DuplexMode::DuplexLongSide:
            return "DuplexNoTumble";
        case PrinterEnum::DuplexMode::DuplexNone:
        default:
            return "None";
        }
    }

    static const QString duplexModeToUIString(const PrinterEnum::DuplexMode &mode)
    {
        switch (mode) {
        case PrinterEnum::DuplexMode::DuplexShortSide:
            return __("Short Edge (Flip)");
        case PrinterEnum::DuplexMode::DuplexLongSide:
            return __("Long Edge (Standard)");
        case PrinterEnum::DuplexMode::DuplexNone:
        default:
            return __("One Sided");
        }
    }

    static PrinterEnum::DuplexMode qDuplexModeToDuplexMode(const QPrinter::DuplexMode &mode)
    {
        switch(mode) {
        case QPrinter::DuplexAuto:
        case QPrinter::DuplexLongSide:
            return PrinterEnum::DuplexMode::DuplexLongSide;
        case QPrinter::DuplexShortSide:
            return PrinterEnum::DuplexMode::DuplexShortSide;
        case QPrinter::DuplexNone:
        default:
            return PrinterEnum::DuplexMode::DuplexNone;
        }
    }

    static ColorModel parsePpdColorModel(const QString &name, const QString &text,
                                         const QString &optionName)
    {
        ColorModel ret;
        ret.name = name;
        ret.text = text;
        ret.originalOption = optionName;

        if (ret.name.contains("Gray") || ret.name.contains("Black")) {
            ret.colorType = PrinterEnum::ColorModelType::GrayType;
        } else {
            ret.colorType = PrinterEnum::ColorModelType::ColorType;
        }
        return ret;
    }

    static PrintQuality parsePpdPrintQuality(const QString &choice,
                                             const QString &text,
                                             const QString &optionName)
    {
        PrintQuality quality;
        quality.name = choice;
        quality.text = text;
        quality.originalOption = optionName;
        return quality;
    }
};

#endif // USC_PRINTERS_UTILS_H
