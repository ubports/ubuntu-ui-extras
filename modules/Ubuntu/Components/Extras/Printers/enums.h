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

#ifndef USC_PRINTERS_ENUMS_H
#define USC_PRINTERS_ENUMS_H

#include "printers_global.h"

#include <QtCore/QObject>

class PRINTERS_DECL_EXPORT PrinterEnum
{
    Q_GADGET

public:
    enum class AccessControl
    {
        AccessAllow = 0,
        AccessDeny,
    };
    Q_ENUM(AccessControl)

    enum class ColorModelType
    {
        GrayType = 0,
        ColorType,
        UnknownType,
    };
    Q_ENUM(ColorModelType)

    enum class DuplexMode
    {
        DuplexNone = 0,
        DuplexLongSide,
        DuplexShortSide,
    };
    Q_ENUM(DuplexMode)

    enum class ErrorPolicy
    {
        RetryOnError = 0,
        AbortOnError,
        StopPrinterOnError,
        RetryCurrentOnError,
    };
    Q_ENUM(ErrorPolicy)

    // Match enums from ipp_jstate_t
    enum class JobState
    {
        Pending = 3,
        Held,
        Processing,
        Stopped,
        Canceled,
        Aborted,
        Complete,
    };
    Q_ENUM(JobState)

    enum class OperationPolicy
    {
        DefaultOperation = 0,
        AuthenticatedOperation,
    };
    Q_ENUM(OperationPolicy)

    enum class PrintRange
    {
        AllPages = 0,
        PageRange,
    };
    Q_ENUM(PrintRange)

    enum class State
    {
        IdleState = 0,
        ActiveState,
        AbortedState,
        ErrorState,
    };
    Q_ENUM(State)

    enum class PrinterType
    {
        ProxyType = 0, // Represents a printer not yet loaded.
        CupsType,
        PdfType,
    };
    Q_ENUM(PrinterType)

    enum class DeviceType
    {
        UnknownType = 0,
        LPDType,
        IppSType,
        Ipp14Type,
        HttpType,
        BehType,
        SocketType,
        HttpsType,
        IppType,
        HPType,
        USBType,
        HPFaxType,
        DNSSDType,
    };
    Q_ENUM(DeviceType)
};

inline uint qHash(const PrinterEnum::JobState &state, uint seed)
{
    return qHash((int) state, seed);
}

#endif // USC_PRINTERS_ENUMS_H
