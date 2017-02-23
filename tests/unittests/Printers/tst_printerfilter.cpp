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
    void testMoveOnPrintersLoaded()
    {
        QScopedPointer<PrinterBackend> backend(new MockPrinterBackend);
        PrinterModel *model = new PrinterModel(backend.data());

        // Setup two printers
        PrinterBackend* printerCBackend = new MockPrinterBackend("c-printer");
        auto printerC = QSharedPointer<Printer>(new Printer(printerCBackend));
        PrinterBackend* printerDBackend = new MockPrinterBackend("d-printer");
        auto printerD = QSharedPointer<Printer>(new Printer(printerDBackend));

        // Setup a filter
        PrinterFilter *filter = new PrinterFilter();
        filter->setSourceModel(model);
        filter->setSortRole(PrinterModel::Roles::DefaultPrinterRole);
        filter->sort(0, Qt::DescendingOrder);

        // Load the two printers
        ((MockPrinterBackend*) backend.data())->mockPrinterLoaded(printerC);
        ((MockPrinterBackend*) backend.data())->mockPrinterLoaded(printerD);

        QCOMPARE(filter->count(), 3);

        // Check the printer names before change
        QCOMPARE(filter->get(0).value("name").toString(), QStringLiteral("c-printer"));
        QCOMPARE(filter->get(1).value("name").toString(), QStringLiteral("d-printer"));
        QCOMPARE(filter->get(2).value("name").toString(), QStringLiteral("Create PDF"));

        // Setup spy
        QSignalSpy dataSpy(filter, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));

        // Change the name of the 2nd printer so it is 1st
        printerDBackend->setPrinterNameInternal("a-printer");

        QModelIndex idx = model->index(1);
        model->dataChanged(idx, idx);

        QTRY_COMPARE(dataSpy.count(), 1);

        // Check the printer names after change
        QCOMPARE(filter->get(0).value("name").toString(), QStringLiteral("a-printer"));
        QCOMPARE(filter->get(1).value("name").toString(), QStringLiteral("c-printer"));
        QCOMPARE(filter->get(2).value("name").toString(), QStringLiteral("Create PDF"));

        delete model;
    }
};

QTEST_GUILESS_MAIN(TestPrinterFilter)
#include "tst_printerfilter.moc"

