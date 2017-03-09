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

#ifndef USC_SIGNAL_RATE_LIMITER_H
#define USC_SIGNAL_RATE_LIMITER_H

#include "printers_global.h"

#include <QDateTime>
#include <QObject>
#include <QSet>
#include <QTimer>

class PRINTERS_DECL_EXPORT SignalRateLimiter : public QObject
{
    Q_OBJECT
    QTimer m_timer;
    QSet<QString> m_unprocessed;
    QDateTime m_unprocessed_time;
public:
    explicit SignalRateLimiter(int triggerEventDelay = 500,
                                  QObject *parent = Q_NULLPTR);
    ~SignalRateLimiter();

public Q_SLOTS:
    void onPrinterModified(
        const QString &text, const QString &printerUri,
        const QString &printerName, uint printerState,
        const QString &printerStateReason, bool acceptingJobs
    );
    void onPrinterStateChanged(
        const QString &text, const QString &printerUri,
        const QString &printerName, uint printerState,
        const QString &printerStateReason, bool acceptingJobs
    );

private Q_SLOTS:
    void process();

Q_SIGNALS:
    void printerModified(const QString &printerName);
};

#endif // USC_SIGNAL_RATE_LIMITERR_H
