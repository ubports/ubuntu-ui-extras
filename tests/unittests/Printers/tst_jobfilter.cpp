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

#include "mockbackend.h"

#include "models/jobmodel.h"

#include <QObject>
#include <QSignalSpy>
#include <QScopedPointer>
#include <QTest>

class TestJobFilter : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFilterOnPrinter()
    {
        QScopedPointer<MockPrinterBackend> backend(new MockPrinterBackend);
        JobModel model(backend.data());

        auto job = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", backend.data()));
        backend->m_jobs << job;
        // Trigger update.
        backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);

        JobFilter filter;
        filter.setSourceModel(&model);

        QCOMPARE(filter.count(), 1);

        // Install filter.
        filter.filterOnPrinterName("other-printer");
        QCOMPARE(filter.count(), 0);
        filter.filterOnPrinterName("test-printer");
        QCOMPARE(filter.count(), 1);
    }
};

QTEST_GUILESS_MAIN(TestJobFilter)
#include "tst_jobfilter.moc"

