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
#include "models/printermodel.h"
#include "printer/printer.h"
#include "printer/printerjob.h"

#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(PrinterBackend*)
Q_DECLARE_METATYPE(PrinterJob*)

class TestPrinterModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_backend = new MockPrinterBackend();
        m_model = new PrinterModel(m_backend);
    }
    void cleanup()
    {
        QSignalSpy destroyedSpy(m_model, SIGNAL(destroyed(QObject*)));
        m_model->deleteLater();
        QTRY_COMPARE(destroyedSpy.count(), 1);
        delete m_backend;
    }

    // Tests for adding/removing/changing things in the model

    void testInsertOnPrintersLoaded()
    {
        QSignalSpy countSpy(m_model, SIGNAL(countChanged()));
        QSignalSpy insertSpy(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)));

        PrinterBackend* printerABackend = new MockPrinterBackend("a-printer");
        auto printerA = QSharedPointer<Printer>(new Printer(printerABackend));

        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(countSpy.count(), 1);
        QCOMPARE(insertSpy.count(), 1);

        // Check item was inserted at end
        QList<QVariant> args = insertSpy.takeFirst();
        QCOMPARE(args.at(1).toInt(), 1);
        QCOMPARE(args.at(2).toInt(), 1);
    }
    void testInsertOnPrinterAddedFromNotification()
    {
        QSignalSpy countSpy(m_model, SIGNAL(countChanged()));
        QSignalSpy insertSpy(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)));

        PrinterBackend* printerABackend = new MockPrinterBackend("a-printer");
        auto printerA = QSharedPointer<Printer>(new Printer(printerABackend));

        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(countSpy.count(), 1);
        QCOMPARE(insertSpy.count(), 1);

        // Check item was inserted at end
        QList<QVariant> args = insertSpy.takeFirst();
        QCOMPARE(args.at(1).toInt(), 1);
        QCOMPARE(args.at(2).toInt(), 1);
    }
    void testRemoveOnPrinterDeletedFromNotification()
    {
        // Setup two printers
        PrinterBackend *printerABackend = new MockPrinterBackend("a-printer");
        auto printerA = QSharedPointer<Printer>(new Printer(printerABackend));
        PrinterBackend *printerBBackend = new MockPrinterBackend("b-printer");
        auto printerB = QSharedPointer<Printer>(new Printer(printerBBackend));

        m_backend->mockPrinterLoaded(printerA);
        m_backend->mockPrinterLoaded(printerB);

        QCOMPARE(m_model->count(), 3);

        // Setup spy and remove a printer
        QSignalSpy countSpy(m_model, SIGNAL(countChanged()));
        QSignalSpy removeSpy(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)));

        m_backend->mockPrinterDeleted("Test removed printer", "", printerB->name(), 0, "", true);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(countSpy.count(), 1);
        QCOMPARE(removeSpy.count(), 1);

        // Check item was removed from end
        QList<QVariant> args = removeSpy.at(0);
        QCOMPARE(args.at(1).toInt(), 2);
        QCOMPARE(args.at(2).toInt(), 2);
    }
    void testUpdateRemoveMulti()
    {
        // Setup four printers
        MockPrinterBackend *printerABackend = new MockPrinterBackend("a-printer");
        auto printerA = QSharedPointer<Printer>(new Printer(printerABackend));
        MockPrinterBackend *printerBBackend = new MockPrinterBackend("b-printer");
        auto printerB = QSharedPointer<Printer>(new Printer(printerBBackend));
        MockPrinterBackend *printerCBackend = new MockPrinterBackend("c-printer");
        auto printerC = QSharedPointer<Printer>(new Printer(printerCBackend));
        MockPrinterBackend *printerDBackend = new MockPrinterBackend("d-printer");
        auto printerD = QSharedPointer<Printer>(new Printer(printerDBackend));

        m_backend->mockPrinterLoaded(printerA);
        m_backend->mockPrinterLoaded(printerB);
        m_backend->mockPrinterLoaded(printerC);
        m_backend->mockPrinterLoaded(printerD);

        QCOMPARE(m_model->count(), 5);

        // Setup spy and remove middle two printers
        QSignalSpy countSpy(m_model, SIGNAL(countChanged()));
        QSignalSpy removeSpy(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)));

        m_backend->mockPrinterDeleted("Test removed printer", "", printerB->name(), 0, "", true);
        m_backend->mockPrinterDeleted("Test removed printer", "", printerC->name(), 0, "", true);

        QCOMPARE(countSpy.count(), 2);
        QCOMPARE(m_model->count(), 3);
        QCOMPARE(removeSpy.count(), 2);

        // Check items were removed from the middle
        QList<QVariant> args;
        args = removeSpy.at(0);
        QCOMPARE(args.at(1).toInt(), 2);
        QCOMPARE(args.at(2).toInt(), 2);

        args = removeSpy.at(1);
        QCOMPARE(args.at(1).toInt(), 2);
        QCOMPARE(args.at(2).toInt(), 2);
    }

    // Tests for the roles in the model exposed to QML

    void testColorModelRole()
    {
        ColorModel a;
        a.text = "Gray";

        ColorModel b;
        b.text = "RGB";
        QList<ColorModel> models({a, b});

        PrinterBackend* backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->printerOptions["a-printer"].insert(
            "DefaultColorModel", QVariant::fromValue(b)
        );
        ((MockPrinterBackend*) backend)->printerOptions["a-printer"].insert(
            "SupportedColorModels", QVariant::fromValue(models)
        );

        auto printerA = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::ColorModelRole).toInt(), models.indexOf(b));
    }
    void testSupportedColorModesRole()
    {
        ColorModel a;
        a.name = "KGray";
        a.text = "Gray";

        ColorModel b;
        b.name = "RGB";
        b.text = "";
        QList<ColorModel> models({a, b});

        PrinterBackend* backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->printerOptions["a-printer"].insert(
            "SupportedColorModels", QVariant::fromValue(models)
        );

        auto printerA = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::SupportedColorModelsRole).toStringList(),
                 (QStringList() << a.text << b.name));
    }
    void testDefaultPrinterRole()
    {
        PrinterBackend* backendA = new MockPrinterBackend("a-printer");
        auto printerA = QSharedPointer<Printer>(new Printer(backendA));
        m_backend->mockPrinterLoaded(printerA);

        PrinterBackend* backendB = new MockPrinterBackend("b-printer");
        auto printerB = QSharedPointer<Printer>(new Printer(backendB));
        m_backend->mockPrinterLoaded(printerB);

        m_backend->m_defaultPrinterName = "a-printer";

        QCOMPARE(m_model->count(), 3);
        QCOMPARE(m_model->data(m_model->index(0), PrinterModel::DefaultPrinterRole).toBool(), false);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::DefaultPrinterRole).toBool(), true);
        QCOMPARE(m_model->data(m_model->index(2), PrinterModel::DefaultPrinterRole).toBool(), false);
    }
    void testDuplexRole()
    {
        QList<PrinterEnum::DuplexMode> modes({
            PrinterEnum::DuplexMode::DuplexNone,
            PrinterEnum::DuplexMode::DuplexLongSide,
            PrinterEnum::DuplexMode::DuplexShortSide,
        });

        PrinterBackend* backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->m_supportedDuplexModes = modes;
        ((MockPrinterBackend*) backend)->printerOptions["a-printer"].insert(
            "Duplex", QVariant::fromValue(Utils::duplexModeToPpdChoice(PrinterEnum::DuplexMode::DuplexLongSide))
        );

        auto printerA = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::DuplexRole).toInt(), 1);
    }
    void testSupportedDuplexModesRole()
    {
        QList<PrinterEnum::DuplexMode> modes({
            PrinterEnum::DuplexMode::DuplexNone,
            PrinterEnum::DuplexMode::DuplexLongSide,
            PrinterEnum::DuplexMode::DuplexShortSide,
        });

        PrinterBackend* backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->m_supportedDuplexModes = modes;

        auto printerA = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::SupportedDuplexModesRole).toStringList(),
                 QStringList()
                    << Utils::duplexModeToUIString(modes.at(0))
                    << Utils::duplexModeToUIString(modes.at(1))
                    << Utils::duplexModeToUIString(modes.at(2)));
    }
    void testNameRole()
    {
        PrinterBackend* backend = new MockPrinterBackend("a-printer");

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::NameRole).toString(),
                 QString("a-printer"));
    }
    void testMakeRole()
    {
        PrinterBackend* backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->m_makeAndModel = "make-and-model";

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::MakeRole).toString(),
                 QString("make-and-model"));
    }
    void testDeviceUriRole()
    {
        MockPrinterBackend* backend = new MockPrinterBackend("a-printer");
        backend->printerOptions["a-printer"].insert(
            "DeviceUri", "/dev/null"
        );

        auto printerA = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::DeviceUriRole).toString(),
                 (QString) "/dev/null");
    }
    void testLastMessageRole()
    {
        MockPrinterBackend* backend = new MockPrinterBackend("a-printer");
        backend->printerOptions["a-printer"].insert(
            "StateMessage", "died"
        );

        auto printerA = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::LastMessageRole).toString(),
                 (QString) "died");
    }
    void testLocationRole()
    {
        PrinterBackend* backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->m_location = "test-location";

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::LocationRole).toString(),
                 QString("test-location"));
    }
    void testEnabledRole()
    {
        PrinterBackend* backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->m_state = PrinterEnum::State::ErrorState;

        auto printerA = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), PrinterModel::EnabledRole).toBool(),
                 true);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::EnabledRole).toBool(),
                 false);
    }
    void testAcceptJobsRole()
    {
        PrinterBackend* backendA = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backendA)->printerOptions["a-printer"].insert(
            "AcceptJobs", QVariant::fromValue(false)
        );

        auto printerA = QSharedPointer<Printer>(new Printer(backendA));
        m_backend->mockPrinterLoaded(printerA);

        PrinterBackend* backendB = new MockPrinterBackend("b-printer");
        ((MockPrinterBackend*) backendB)->printerOptions["b-printer"].insert(
            "AcceptJobs", QVariant::fromValue(true)
        );

        auto printerB = QSharedPointer<Printer>(new Printer(backendB));
        m_backend->mockPrinterLoaded(printerB);

        QCOMPARE(m_model->count(), 3);
        QCOMPARE(m_model->data(m_model->index(0), PrinterModel::AcceptJobsRole).toBool(),
                 true);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::AcceptJobsRole).toBool(),
                 false);
        QCOMPARE(m_model->data(m_model->index(2), PrinterModel::AcceptJobsRole).toBool(),
                 true);
    }
    void testSharedRole()
    {
        MockPrinterBackend* backendA = new MockPrinterBackend("a-printer");
        backendA->printerOptions["a-printer"].insert("Shared", false);

        auto printerA = QSharedPointer<Printer>(new Printer(backendA));
        m_backend->mockPrinterLoaded(printerA);

        MockPrinterBackend* backendB = new MockPrinterBackend("b-printer");
        backendB->printerOptions["b-printer"].insert("Shared", true);

        auto printerB = QSharedPointer<Printer>(new Printer(backendB));
        m_backend->mockPrinterLoaded(printerB);

        QCOMPARE(m_model->data(m_model->index(0), PrinterModel::SharedRole).toBool(),
                 false);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::SharedRole).toBool(),
                 false);
        QCOMPARE(m_model->data(m_model->index(2), PrinterModel::SharedRole).toBool(),
                 true);
    }
    void testPrintQualityRole()
    {
        PrintQuality a;
        a.name = "Poor";

        PrintQuality b;
        b.name = "Worse";
        QList<PrintQuality> qualities({a, b});

        PrinterBackend *backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->printerOptions["a-printer"].insert(
            "SupportedPrintQualities", QVariant::fromValue(qualities)
        );
        ((MockPrinterBackend*) backend)->printerOptions["a-printer"].insert(
            "DefaultPrintQuality", QVariant::fromValue(b)
        );

        auto printerA = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::PrintQualityRole).toInt(), qualities.indexOf(b));
    }
    void testSupportedPrintQualitiesRole()
    {
        PrintQuality a;
        a.name = "fast-draft";
        a.text = "Draft";

        PrintQuality b;
        b.name = "normal";
        b.text = "";
        QList<PrintQuality> qualities({a, b});

        PrinterBackend *backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->printerOptions["a-printer"].insert(
            "SupportedPrintQualities", QVariant::fromValue(qualities)
        );

        auto printerA = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printerA);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::SupportedPrintQualitiesRole).toStringList(),
                 (QStringList() << a.text << b.name));
    }
    void testDescriptionRole()
    {
        PrinterBackend* backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->m_description = "test-description";

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::DescriptionRole).toString(),
                 QString("test-description"));
    }
    void testPageSizeRole()
    {
        QPageSize size = QPageSize(QPageSize::A4);

        PrinterBackend *backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->m_defaultPageSize = size;

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::PageSizeRole).toString(),
                 size.name());
    }
    void testSupportedPageSizesRole()
    {
        QList<QPageSize> sizes({
            QPageSize(QPageSize::A4),
            QPageSize(QPageSize::A5),
        });

        PrinterBackend *backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->m_supportedPageSizes = sizes;

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::SupportedPageSizesRole).toStringList(),
                 QStringList() << sizes.at(0).name() << sizes.at(1).name());
    }
    void testStateRole()
    {
        PrinterBackend *backend = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backend)->m_state = PrinterEnum::State::ActiveState;

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), PrinterModel::StateRole).value<PrinterEnum::State>(),
                 PrinterEnum::State::IdleState);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::StateRole).value<PrinterEnum::State>(),
                 PrinterEnum::State::ActiveState);
    }
    void testPrinterRole()
    {
        PrinterBackend *backend = new MockPrinterBackend("a-printer");

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        QCOMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::PrinterRole).value<QSharedPointer<Printer>>(),
                 printer);
    }
    void testIsPdfRole()
    {
        PrinterBackend* backendA = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backendA)->m_type = PrinterEnum::PrinterType::PdfType;

        auto printerA = QSharedPointer<Printer>(new Printer(backendA));
        m_backend->mockPrinterLoaded(printerA);

        PrinterBackend* backendB = new MockPrinterBackend("b-printer");
        ((MockPrinterBackend*) backendB)->m_type = PrinterEnum::PrinterType::CupsType;

        auto printerB = QSharedPointer<Printer>(new Printer(backendB));
        m_backend->mockPrinterLoaded(printerB);

        PrinterBackend* backendC = new MockPrinterBackend("c-printer");
        ((MockPrinterBackend*) backendC)->m_type = PrinterEnum::PrinterType::ProxyType;

        auto printerC = QSharedPointer<Printer>(new Printer(backendC));
        m_backend->mockPrinterLoaded(printerC);

        QCOMPARE(m_model->count(), 4);
        // First printer is the pdf printer ("Create PDF")
        QCOMPARE(m_model->data(m_model->index(0), PrinterModel::IsPdfRole).toBool(),
                 true);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::IsPdfRole).toBool(),
                 true);
        QCOMPARE(m_model->data(m_model->index(2), PrinterModel::IsPdfRole).toBool(),
                 false);
        QCOMPARE(m_model->data(m_model->index(3), PrinterModel::IsPdfRole).toBool(),
                 false);
    }
    void testIsLoadedRole()
    {
        PrinterBackend* backendA = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backendA)->m_type = PrinterEnum::PrinterType::PdfType;

        auto printerA = QSharedPointer<Printer>(new Printer(backendA));
        m_backend->mockPrinterLoaded(printerA);

        PrinterBackend* backendB = new MockPrinterBackend("b-printer");
        ((MockPrinterBackend*) backendB)->m_type = PrinterEnum::PrinterType::CupsType;

        auto printerB = QSharedPointer<Printer>(new Printer(backendB));
        m_backend->mockPrinterLoaded(printerB);

        PrinterBackend* backendC = new MockPrinterBackend("c-printer");
        ((MockPrinterBackend*) backendC)->m_type = PrinterEnum::PrinterType::ProxyType;

        auto printerC = QSharedPointer<Printer>(new Printer(backendC));
        m_backend->mockPrinterLoaded(printerC);

        QCOMPARE(m_model->count(), 4);
        // First printer is the pdf printer ("Create PDF")
        QCOMPARE(m_model->data(m_model->index(0), PrinterModel::IsLoadedRole).toBool(),
                 true);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::IsLoadedRole).toBool(),
                 true);
        QCOMPARE(m_model->data(m_model->index(2), PrinterModel::IsLoadedRole).toBool(),
                 true);
        QCOMPARE(m_model->data(m_model->index(3), PrinterModel::IsLoadedRole).toBool(),
                 false);
    }
    void testIsRawRole()
    {
        PrinterBackend* backendA = new MockPrinterBackend("a-printer");
        ((MockPrinterBackend*) backendA)->m_holdsDefinition = false;

        auto printerA = QSharedPointer<Printer>(new Printer(backendA));
        m_backend->mockPrinterLoaded(printerA);

        PrinterBackend* backendB = new MockPrinterBackend("b-printer");
        ((MockPrinterBackend*) backendB)->m_holdsDefinition = true;

        auto printerB = QSharedPointer<Printer>(new Printer(backendB));
        m_backend->mockPrinterLoaded(printerB);

        QCOMPARE(m_model->count(), 3);
        QCOMPARE(m_model->data(m_model->index(0), PrinterModel::IsRawRole).toBool(),
                 true);
        QCOMPARE(m_model->data(m_model->index(1), PrinterModel::IsRawRole).toBool(),
                 true);
        QCOMPARE(m_model->data(m_model->index(2), PrinterModel::IsRawRole).toBool(),
                 false);
    }
    void testJobRole()
    {
        PrinterBackend* backend = new MockPrinterBackend("a-printer");

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        QCOMPARE(m_model->count(), 2);

        auto job = m_model->data(m_model->index(0), PrinterModel::JobRole).value<QAbstractItemModel *>();

        QVERIFY(job != Q_NULLPTR);
        QCOMPARE(job->rowCount(), 0);
    }
    void testCopiesRole()
    {
        MockPrinterBackend* backend = new MockPrinterBackend("a-printer");
        backend->printerOptions["a-printer"].insert("Copies", "2");

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        auto copies = m_model->data(m_model->index(1), PrinterModel::CopiesRole).toInt();
        QCOMPARE(copies, 2);

        m_model->setData(m_model->index(1), 5, PrinterModel::CopiesRole);
        QCOMPARE(backend->printerOptions["a-printer"].value("Copies").toInt(), 5);
    }

private:
    MockPrinterBackend *m_backend;
    PrinterModel *m_model;
};

QTEST_GUILESS_MAIN(TestPrinterModel)
#include "tst_printermodel.moc"

