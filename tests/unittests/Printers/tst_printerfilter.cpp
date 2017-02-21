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

#include "models/printermodel.h"

#include <QObject>
#include <QSignalSpy>
#include <QScopedPointer>
#include <QTest>

Q_DECLARE_METATYPE(PrinterBackend*)
Q_DECLARE_METATYPE(PrinterJob*)

class TestPrinterFilter : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEmptyCount()
    {
        QScopedPointer<PrinterBackend> backend(new MockPrinterBackend);
        PrinterModel model(backend.data());

        PrinterFilter filter;
        filter.setSourceModel(&model);
        QCOMPARE(filter.count(), model.count());
    }
    void testNonEmptyCount()
    {
        QScopedPointer<PrinterBackend> backend(new MockPrinterBackend);
        PrinterModel model(backend.data());

        PrinterBackend* printerABackend = new MockPrinterBackend("a-printer");
        PrinterBackend* printerBBackend = new MockPrinterBackend("b-printer");
        auto printerA = QSharedPointer<Printer>(new Printer(printerABackend));
        auto printerB = QSharedPointer<Printer>(new Printer(printerBBackend));

        PrinterFilter filter;
        filter.setSourceModel(&model);
        ((MockPrinterBackend*) backend.data())->mockPrinterLoaded(printerA);
        ((MockPrinterBackend*) backend.data())->mockPrinterLoaded(printerB);

        QCOMPARE(filter.count(), 3);
    }
    void testCountChanged()
    {
        QScopedPointer<PrinterBackend> backend(new MockPrinterBackend);
        PrinterModel model(backend.data());

        PrinterBackend* printerABackend = new MockPrinterBackend("a-printer");
        auto printerA = QSharedPointer<Printer>(new Printer(printerABackend));

        PrinterFilter filter;
        filter.setSourceModel(&model);

        QSignalSpy modelCountSpy(&model, SIGNAL(countChanged()));
        QSignalSpy filterCountSpy(&filter, SIGNAL(countChanged()));
        ((MockPrinterBackend*) backend.data())->mockPrinterLoaded(printerA);
        QCOMPARE(modelCountSpy.count(), 1);
        QCOMPARE(filterCountSpy.count(), 1);
    }
    // void testMoveOnPrintersLoaded()
    // {
    //     // Setup two printers
    //     PrinterBackend* printerABackend = new MockPrinterBackend("a-printer");
    //     auto printerA = QSharedPointer<Printer>(new Printer(printerABackend));
    //     PrinterBackend* printerBBackend = new MockPrinterBackend("b-printer");
    //     auto printerB = QSharedPointer<Printer>(new Printer(printerBBackend));

    //     getBackend()->m_availablePrinterNames << printerA->name() << printerB->name();

    //     getBackend()->mockPrinterLoaded(printerA);
    //     getBackend()->mockPrinterLoaded(printerB);

    //     QCOMPARE(m_model->count(), 2);

    //     // Setup spy and move a printer
    //     int from = 1;
    //     int to = 0;
    //     qWarning() << "before move" << getBackend()->m_availablePrinterNames;
    //     getBackend()->m_availablePrinterNames.move(from, to);
    //     qWarning() << "after move" << getBackend()->m_availablePrinterNames;

    //     // Check signals were fired
    //     QSignalSpy moveSpy(m_model, SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)));

    //     // If a printer is re-named, we would expect a move signal.
    //     getBackend()->mockPrinterModified("Test renamed printer", "", printerB->name(), 0, "", true);

    //     QCOMPARE(moveSpy.count(), 1);
    //     QCOMPARE(m_model->count(), 2);

    //     // Check item was moved from -> to
    //     QList<QVariant> args = moveSpy.at(0);
    //     QCOMPARE(args.at(1).toInt(), from);
    //     QCOMPARE(args.at(2).toInt(), from);
    //     QCOMPARE(args.at(4).toInt(), to);
    // }
};

QTEST_GUILESS_MAIN(TestPrinterFilter)
#include "tst_printerfilter.moc"

