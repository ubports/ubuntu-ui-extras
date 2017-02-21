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

#include "backend/backend.h"
#include "printers/printers.h"

#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(PrinterBackend*)
Q_DECLARE_METATYPE(QList<QSharedPointer<Printer>>)

class TestPrinters : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testInstantiation_data()
    {
        QTest::addColumn<PrinterBackend*>("backend");

        {
            PrinterBackend* backend = new MockPrinterBackend;
            QTest::newRow("no printers") << backend;
        }
    }
    void testInstantiation()
    {
        QFETCH(PrinterBackend*, backend);
        Printers printers(backend);
    }
    void testAllPrintersFilter_data()
    {
        QTest::addColumn<QStringList>("in");
        QTest::addColumn<QStringList>("out");
        QTest::addColumn<QString>("defaultPrinterName");
        {
            auto in = QStringList({"printer-a", "printer-b"});
            auto out = QStringList({"printer-a", "printer-b"});

            QTest::newRow("no defaults") << in << out << "";
        }
        {
            auto in = QStringList({"printer-a", "printer-b"});
            auto out = QStringList({"printer-b", "printer-a"});
            QTest::newRow("have default") << in << out << "printer-b";
        }
    }
    void testAllPrintersFilter()
    {
        QFETCH(QStringList, in);
        QFETCH(QStringList, out);
        QFETCH(QString, defaultPrinterName);

        MockPrinterBackend* backend = new MockPrinterBackend;
        backend->m_defaultPrinterName = defaultPrinterName;
        Q_FOREACH(auto existingPrinter, in) {
            backend->m_availablePrinterNames << existingPrinter;
        }
        Printers printers(backend);

        auto all = printers.allPrinters();

        QCOMPARE(all->rowCount(), out.size());
        for (int i = 0; i < all->rowCount(); i++) {
            QCOMPARE(
                 all->data(all->index(i, 0)).toString(),
                 out.at(i)
            );
        }
    }
    void testPrinterDrivers()
    {
        QString targetFilter("foo");
        Printers printers(new MockPrinterBackend);
        printers.setDriverFilter(targetFilter);

        DriverModel *drivers = (DriverModel*) printers.drivers();
        QCOMPARE(drivers->filter(), targetFilter);
    }
    void testCreateJob()
    {

    }
    void testCancelJob()
    {

    }
    void testPrinterRemove()
    {
        // TODO
    }
    void testSetDefault()
    {

    }
    /* Test that Printers successfully assigns printers to jobs whenever
    they appear, as well as assigning job proxies to printers whenever they
    appear. */
    void testAssignPrinterToJob()
    {
        MockPrinterBackend *backend = new MockPrinterBackend;
        Printers p(backend);

        MockPrinterBackend *printerBackend = new MockPrinterBackend("test-printer");
        auto printer = QSharedPointer<Printer>(new Printer(printerBackend));
        backend->mockPrinterLoaded(printer);

        auto job = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", backend));
        backend->m_jobs << job;

        // Trigger update.
        backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);

        // Job now has a shared pointer to printer.
        QCOMPARE(job->printer()->name(), printer->name());
    }
    void testSetPrinterJobFilter()
    {
        MockPrinterBackend *backend = new MockPrinterBackend;
        Printers p(backend);

        auto job = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", backend));
        backend->m_jobs << job;
        backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);

        MockPrinterBackend *printerBackend = new MockPrinterBackend("test-printer");
        auto printer = QSharedPointer<Printer>(new Printer(printerBackend));
        backend->mockPrinterLoaded(printer);

        QCOMPARE(printer->jobs()->rowCount(), 1);

        // Need to also get this through a model.
        auto printerJobs = p.allPrinters()->data(p.allPrinters()->index(0,0), PrinterModel::Roles::JobRole).value<QAbstractItemModel*>();
        QCOMPARE(printerJobs->rowCount(), 1);
    }
};

QTEST_GUILESS_MAIN(TestPrinters)
#include "tst_printers.moc"
