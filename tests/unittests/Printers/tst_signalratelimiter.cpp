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

#include "printer/signalratelimiter.h"

#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

class TestSignalRateLimiter : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEmptyCount()
    {
        SignalRateLimiter handler(500);
        QSignalSpy modifiedSpy(&handler, SIGNAL(printerModified(const QString&)));

        for (int i = 0; i < 20; i++) {
            handler.onPrinterStateChanged("spam!", "ipp://bar/baz", "printer-a", 0, "none", true);
        }

        modifiedSpy.wait(1000);
        QCOMPARE(modifiedSpy.count(), 1);
    }
    void testUnprocessedTime()
    {
        // Keep sending jobs with no gap for longer than four times the
        // event delay. Check that two signals are emitted.
        // One from the forcing of the signal and one as the timer finishes

        int eventDelay = 200;
        SignalRateLimiter handler(eventDelay);
        QSignalSpy modifiedSpy(&handler, SIGNAL(printerModified(const QString&)));

        QDateTime start = QDateTime::currentDateTime();

        while (start.msecsTo(QDateTime::currentDateTime()) < eventDelay * 5) {
            handler.onPrinterStateChanged("spam!", "ipp://foo/bar", "printer-a", 0, "none", true);
        }

        modifiedSpy.wait(eventDelay * 2);
        QCOMPARE(modifiedSpy.count(), 2);
    }
};

QTEST_GUILESS_MAIN(TestSignalRateLimiter)
#include "tst_signalratelimiter.moc"

