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
        // QSignalSpy destroyedSpy(m_instance, SIGNAL(destroyed(QObject*)));
        // m_instance->deleteLater();
        // QTRY_COMPARE(destroyedSpy.count(), 1);

        // m_instance = new PrinterJob(m_printer_name, m_backend);
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
        QSKIP("Not implemented yet!");

//        ((MockPrinterInfo *) m_mock_printer)->m_defaultColorMode = ColorMode::ColorMode;
//        refreshInstance();
//        QCOMPARE(m_instance->colorMode(), ColorMode::ColorMode);

//        m_instance->setColorMode(ColorMode::GrayscaleMode);
//        QCOMPARE(m_instance->colorMode(), ColorMode::GrayscaleMode);

//        ((MockPrinterInfo *) m_mock_printer)->m_defaultColorMode = ColorMode::GrayscaleMode;
//        refreshInstance();
//        QCOMPARE(m_instance->colorMode(), ColorMode::GrayscaleMode);

//        m_instance->setColorMode(ColorMode::ColorMode);
//        QCOMPARE(m_instance->colorMode(), ColorMode::ColorMode);
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
        QSKIP("Not implemented yet!");
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
        QSKIP("Not implemented yet!");
    }

    void testReverse()
    {
        QCOMPARE(m_instance->reverse(), false);
        m_instance->setReverse(true);
        QCOMPARE(m_instance->reverse(), true);
    }

    void testState()
    {
        QSKIP("Not implemented yet!");
    }
private:
    PrinterJob *m_instance = nullptr;
    MockPrinterBackend *m_backend = nullptr;
    QSharedPointer<Printer> m_mock_printer;
    QString m_printer_name = "my-printer";
};

QTEST_GUILESS_MAIN(TestPrinterJob)
#include "tst_printerjob.moc"

