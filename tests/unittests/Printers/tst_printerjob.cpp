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

#include "utils.h"

#include "mockbackend.h"

#include "backend/backend.h"
#include "printer/printer.h"
#include "printer/printerjob.h"

#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(QList<QPageSize>)

class TestPrinterJob : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_backend = new MockPrinterBackend(m_printer_name);
        m_mock_printer = QSharedPointer<Printer>(new Printer(m_backend));
        m_instance = new PrinterJob(m_printer_name, m_backend);
        m_instance->setPrinter(m_mock_printer);
    }
    void cleanup()
    {
        QSignalSpy destroyedJobSpy(m_instance, SIGNAL(destroyed(QObject*)));
        m_instance->deleteLater();
        QTRY_COMPARE(destroyedJobSpy.count(), 1);

        QSignalSpy destroyedSpy(m_mock_printer.data(), SIGNAL(destroyed(QObject*)));
        m_mock_printer.clear();
        QTRY_COMPARE(destroyedSpy.count(), 1);

        delete m_backend;
    }
    void refreshInstance()
    {
        QSignalSpy destroyedSpy(m_instance, SIGNAL(destroyed(QObject*)));
        m_instance->deleteLater();
        QTRY_COMPARE(destroyedSpy.count(), 1);

        m_instance = new PrinterJob(m_printer_name, m_backend);
        m_instance->setPrinter(m_mock_printer);
    }
    void refreshInstanceWithBackend(MockPrinterBackend *backend)
    {
        QSignalSpy destroyedJobSpy(m_instance, SIGNAL(destroyed(QObject*)));
        m_instance->deleteLater();
        QTRY_COMPARE(destroyedJobSpy.count(), 1);

        QSignalSpy destroyedSpy(m_mock_printer.data(), SIGNAL(destroyed(QObject*)));
        m_mock_printer.clear();
        QTRY_COMPARE(destroyedSpy.count(), 1);

        m_backend = backend;
        m_mock_printer = QSharedPointer<Printer>(new Printer(m_backend));

        m_instance = new PrinterJob(m_printer_name, m_backend);
        m_instance->setPrinter(m_mock_printer);
    }

    void testCollate()
    {
        QCOMPARE(m_instance->collate(), true);
        m_instance->setCollate(false);
        QCOMPARE(m_instance->collate(), false);
    }

    void testColorModel()
    {
        ColorModel a;
        a.colorType = PrinterEnum::ColorModelType::GrayType;
        a.text = "Gray";

        ColorModel b;
        b.colorType = PrinterEnum::ColorModelType::ColorType;
        b.text = "RGB";
        QList<ColorModel> models({a, b});

        MockPrinterBackend *backend = new MockPrinterBackend(m_printer_name);
        ((MockPrinterBackend*) backend)->printerOptions[m_printer_name].insert(
            "SupportedColorModels", QVariant::fromValue(models));

        refreshInstanceWithBackend(backend);

        // Check the models are the same
        QCOMPARE(m_mock_printer->supportedColorModels(), models);

        // Set colorModel to 0 (Gray)
        m_instance->setColorModel(0);

        QCOMPARE(m_instance->colorModel(), 0);
        QCOMPARE(m_instance->getColorModel(), models.at(0));
        QCOMPARE(m_instance->colorModelType(), models.at(0).colorType);

        // Set colorModel to 1 (RGB)
        m_instance->setColorModel(1);

        QCOMPARE(m_instance->colorModel(), 1);
        QCOMPARE(m_instance->getColorModel(), models.at(1));
        QCOMPARE(m_instance->colorModelType(), models.at(1).colorType);
    }

    void testCopies()
    {
        QCOMPARE(m_instance->copies(), 1);
        m_instance->setCopies(2);
        QCOMPARE(m_instance->copies(), 2);
    }

    void testCopiesLessThanOne()
    {
        QCOMPARE(m_instance->copies(), 1);
        m_instance->setCopies(0);
        QCOMPARE(m_instance->copies(), 1);
        m_instance->setCopies(-1);
        QCOMPARE(m_instance->copies(), 1);
    }

    void testDuplex()
    {
        QList<PrinterEnum::DuplexMode> modes = {
                PrinterEnum::DuplexMode::DuplexNone,
                PrinterEnum::DuplexMode::DuplexLongSide,
                PrinterEnum::DuplexMode::DuplexShortSide
        };
        m_backend->m_supportedDuplexModes = modes;

        m_mock_printer->setDefaultDuplexMode(PrinterEnum::DuplexMode::DuplexNone);
        refreshInstance();
        QCOMPARE(m_instance->duplexMode(), modes.indexOf(PrinterEnum::DuplexMode::DuplexNone));
        QCOMPARE(m_instance->getDuplexMode(), PrinterEnum::DuplexMode::DuplexNone);

        m_instance->setDuplexMode(modes.indexOf(PrinterEnum::DuplexMode::DuplexLongSide));
        QCOMPARE(m_instance->duplexMode(), modes.indexOf(PrinterEnum::DuplexMode::DuplexLongSide));
        QCOMPARE(m_instance->getDuplexMode(), PrinterEnum::DuplexMode::DuplexLongSide);

        m_mock_printer->setDefaultDuplexMode(PrinterEnum::DuplexMode::DuplexShortSide);
        refreshInstance();
        QCOMPARE(m_instance->duplexMode(), modes.indexOf(PrinterEnum::DuplexMode::DuplexShortSide));
        QCOMPARE(m_instance->getDuplexMode(), PrinterEnum::DuplexMode::DuplexShortSide);

        m_instance->setDuplexMode(modes.indexOf(PrinterEnum::DuplexMode::DuplexLongSide));
        QCOMPARE(m_instance->duplexMode(), modes.indexOf(PrinterEnum::DuplexMode::DuplexLongSide));
        QCOMPARE(m_instance->getDuplexMode(), PrinterEnum::DuplexMode::DuplexLongSide);
    }

    void testIsTwoSided()
    {
        QList<PrinterEnum::DuplexMode> modes = {
                PrinterEnum::DuplexMode::DuplexNone,
                PrinterEnum::DuplexMode::DuplexLongSide,
                PrinterEnum::DuplexMode::DuplexShortSide
        };
        m_backend->m_supportedDuplexModes = modes;

        m_mock_printer->setDefaultDuplexMode(PrinterEnum::DuplexMode::DuplexNone);
        refreshInstance();
        QCOMPARE(m_instance->duplexMode(), modes.indexOf(PrinterEnum::DuplexMode::DuplexNone));
        QCOMPARE(m_instance->getDuplexMode(), PrinterEnum::DuplexMode::DuplexNone);
        QCOMPARE(m_instance->isTwoSided(), false);

        m_instance->setDuplexMode(modes.indexOf(PrinterEnum::DuplexMode::DuplexLongSide));

        QCOMPARE(m_instance->duplexMode(), modes.indexOf(PrinterEnum::DuplexMode::DuplexLongSide));
        QCOMPARE(m_instance->getDuplexMode(), PrinterEnum::DuplexMode::DuplexLongSide);
        QCOMPARE(m_instance->isTwoSided(), true);

        m_instance->setDuplexMode(modes.indexOf(PrinterEnum::DuplexMode::DuplexShortSide));

        QCOMPARE(m_instance->duplexMode(), modes.indexOf(PrinterEnum::DuplexMode::DuplexShortSide));
        QCOMPARE(m_instance->getDuplexMode(), PrinterEnum::DuplexMode::DuplexShortSide);
        QCOMPARE(m_instance->isTwoSided(), true);
    }

    void testPrintFile()
    {
        QSignalSpy printFileSpy(m_backend, SIGNAL(printToFile(QString, QString)));

        QString title = QStringLiteral("Title");
        QUrl url("file:///tmp/test.pdf");

        m_instance->setTitle(title);
        m_instance->printFile(url);

        QTRY_COMPARE(printFileSpy.count(), 1);

        QList<QVariant> args = printFileSpy.takeFirst();
        QCOMPARE(args.at(0).toString(), url.toLocalFile());
        QCOMPARE(args.at(1).toString(), title);
    }

    void testPrintRange()
    {
        QCOMPARE(m_instance->printRange(), QStringLiteral(""));
        m_instance->setPrintRange("2-5,7,8");
        QCOMPARE(m_instance->printRange(), QStringLiteral("2-5,7,8"));
    }

    void testPrintRangeMode()
    {
        QCOMPARE(m_instance->printRangeMode(), PrinterEnum::PrintRange::AllPages);
        m_instance->setPrintRangeMode(PrinterEnum::PrintRange::PageRange);
        QCOMPARE(m_instance->printRangeMode(), PrinterEnum::PrintRange::PageRange);
    }

    void testQuality()
    {
        PrintQuality a;
        a.name = "Poor";

        PrintQuality b;
        b.name = "Worse";
        QList<PrintQuality> qualities({a, b});

        MockPrinterBackend *backend = new MockPrinterBackend(m_printer_name);
        backend->printerOptions[m_printer_name].insert(
            "SupportedPrintQualities", QVariant::fromValue(qualities));

        refreshInstanceWithBackend(backend);

        // Check the models are the same
        QCOMPARE(m_mock_printer->supportedPrintQualities(), qualities);

        // Set quality to Poor
        m_instance->setQuality(0);

        QCOMPARE(m_instance->quality(), 0);
        QCOMPARE(m_instance->getPrintQuality(), qualities.at(0));

        // Set quality to Worse
        m_instance->setQuality(1);

        QCOMPARE(m_instance->quality(), 1);
        QCOMPARE(m_instance->getPrintQuality(), qualities.at(1));
    }

    void testReverse()
    {
        QCOMPARE(m_instance->reverse(), false);
        m_instance->setReverse(true);
        QCOMPARE(m_instance->reverse(), true);
    }

    void testSetPrinter()
    {
        // The following properties should not change when using setPrinter
        bool collate = !m_instance->collate();
        int copies = m_instance->copies() + 1;
        bool landscape = !m_instance->landscape();
        QString printRange = QStringLiteral("2-4,6");
        PrinterEnum::PrintRange printRangeMode = PrinterEnum::PrintRange::PageRange;
        bool reverse = !m_instance->reverse();
        QString title = QStringLiteral("my-title");

        m_instance->setCollate(collate);
        m_instance->setCopies(copies);
        m_instance->setLandscape(landscape);
        m_instance->setPrintRange(printRange);
        m_instance->setPrintRangeMode(printRangeMode);
        m_instance->setReverse(reverse);
        m_instance->setTitle(title);

        QCOMPARE(m_instance->collate(), collate);
        QCOMPARE(m_instance->copies(), copies);
        QCOMPARE(m_instance->landscape(), landscape);
        QCOMPARE(m_instance->printRange(), printRange);
        QCOMPARE(m_instance->printRangeMode(), printRangeMode);
        QCOMPARE(m_instance->reverse(), reverse);
        QCOMPARE(m_instance->title(), title);

        MockPrinterBackend *backend = new MockPrinterBackend(m_printer_name);
        QSharedPointer<Printer> printer = QSharedPointer<Printer>(new Printer(backend));

        m_instance->setPrinter(printer);

        QCOMPARE(m_instance->collate(), collate);
        QCOMPARE(m_instance->copies(), copies);
        QCOMPARE(m_instance->landscape(), landscape);
        QCOMPARE(m_instance->printRange(), printRange);
        QCOMPARE(m_instance->printRangeMode(), printRangeMode);
        QCOMPARE(m_instance->reverse(), reverse);
        QCOMPARE(m_instance->title(), title);
    }

    void testState()
    {
        QCOMPARE(m_instance->state(), PrinterEnum::JobState::Pending);
    }

    /* Test that the copies value is changed in certain situations:
        - when the printer changes, and
        - the current copies value matches the default of the previous printer
    */
    void testSaneCopiesValue()
    {
        MockPrinterBackend *backend1 = new MockPrinterBackend("printer1");
        backend1->printerOptions["printer1"].insert("Copies", "2");
        QSharedPointer<Printer> printer1 = QSharedPointer<Printer>(new Printer(backend1));

        MockPrinterBackend *backend2 = new MockPrinterBackend("printer2");
        backend2->printerOptions["printer2"].insert("Copies", "5");
        QSharedPointer<Printer> printer2 = QSharedPointer<Printer>(new Printer(backend2));

        // Base case
        m_instance->setPrinter(printer1);
        QCOMPARE(m_instance->copies(), 2);

        // We expect the job to assume the copies of the new printer
        m_instance->setPrinter(printer2);
        QCOMPARE(m_instance->copies(), 5);

        // Set a non-default value, change printer.
        m_instance->setCopies(100);
        m_instance->setPrinter(printer1);
        QCOMPARE(m_instance->copies(), 100); // Copies stays the same.
    }
private:
    PrinterJob *m_instance = nullptr;
    MockPrinterBackend *m_backend = nullptr;
    QSharedPointer<Printer> m_mock_printer;
    QString m_printer_name = "my-printer";
};

QTEST_GUILESS_MAIN(TestPrinterJob)
#include "tst_printerjob.moc"

