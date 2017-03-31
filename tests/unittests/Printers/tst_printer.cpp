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
#include "backend/backend_pdf.h"
#include "printer/printer.h"

#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(QList<QPageSize>)

class TestPrinter : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_backend = new MockPrinterBackend(m_printerName);
        m_instance = QSharedPointer<Printer>(new Printer(m_backend));
    }
    void cleanup()
    {
        QSignalSpy destroyedSpy(m_instance.data(), SIGNAL(destroyed(QObject*)));
        m_instance.clear();
        QTRY_COMPARE(destroyedSpy.count(), 1);

        if (m_backend) {
            delete m_backend;
        }
    }
    void testName()
    {
        QCOMPARE(m_backend->printerName(), m_instance->name());
    }
    void testDescription()
    {
        m_backend->m_description = "some description";
        QCOMPARE(m_backend->description(), m_instance->description());
    }
    void testSetDescription()
    {
        QString desc("another description");
        m_instance->setDescription(desc);
        QCOMPARE(m_backend->infos.value(m_printerName), desc);
    }
    void testDeviceUri()
    {
        MockPrinterBackend *backend = new MockPrinterBackend(m_printerName);
        backend->printerOptions[m_printerName].insert(
            "DeviceUri", "/dev/null");
        Printer p(backend);
        QCOMPARE(p.deviceUri(), (QString) "/dev/null");
    }
    void testLastMessage()
    {
        MockPrinterBackend *backend = new MockPrinterBackend(m_printerName);
        backend->printerOptions[m_printerName].insert(
            "StateMessage", "died");
        Printer p(backend);
        QCOMPARE(p.lastMessage(), (QString) "died");
    }
    void testMakeModel()
    {
        m_backend->m_makeAndModel = "make and model";
        QCOMPARE(m_instance->make(), m_backend->makeAndModel());
    }
    void testLocation()
    {
        m_backend->m_location = "location";
        QCOMPARE(m_instance->location(), m_backend->location());
    }
    void testCopies()
    {
        MockPrinterBackend *backend = new MockPrinterBackend(m_printerName);
        backend->printerOptions[m_printerName].insert("Copies", "2");
        Printer p(backend);
        QCOMPARE(p.copies(), 2);
    }
    void testSetCopies()
    {
        MockPrinterBackend *backend = new MockPrinterBackend(m_printerName);
        backend->printerOptions[m_printerName].insert("Copies", "2");
        Printer p(backend);
        p.setCopies(5);
        QCOMPARE(backend->printerOptions[m_printerName].value("Copies").toInt(), 5);
    }
    void testRemote()
    {
        m_backend->m_remote = false;
        QCOMPARE(m_instance->isRemote(), m_backend->isRemote());
        m_backend->m_remote = true;
        QCOMPARE(m_instance->isRemote(), m_backend->isRemote());
    }
    void testSupportedDuplexModes_data()
    {
        QTest::addColumn<QList<PrinterEnum::DuplexMode>>("modes");
        QTest::addColumn<QStringList>("strings");
        {
            QList<PrinterEnum::DuplexMode> modes({PrinterEnum::DuplexMode::DuplexNone});
            QStringList strings({"One Sided"});
            QTest::newRow("one") << modes << strings;
        }
        {
            QList<PrinterEnum::DuplexMode> modes({PrinterEnum::DuplexMode::DuplexNone, PrinterEnum::DuplexMode::DuplexLongSide});
            QStringList strings({"One Sided", "Long Edge (Standard)"});
            QTest::newRow("multiple") << modes << strings;
        }
    }
    void testSupportedDuplexModes()
    {
        QFETCH(QList<PrinterEnum::DuplexMode>, modes);
        QFETCH(QStringList, strings);
        m_backend->m_supportedDuplexModes = modes;
        QCOMPARE(m_instance->supportedDuplexModes(), modes);
        QCOMPARE(m_instance->supportedDuplexStrings(), strings);
    }
    void testDefaultDuplexMode()
    {
        QList<PrinterEnum::DuplexMode> modes({
            PrinterEnum::DuplexMode::DuplexNone,
            PrinterEnum::DuplexMode::DuplexLongSide,
            PrinterEnum::DuplexMode::DuplexShortSide});
        m_backend->m_supportedDuplexModes = modes;

        m_instance->setDefaultDuplexMode(PrinterEnum::DuplexMode::DuplexLongSide);
        QCOMPARE(m_instance->defaultDuplexMode(), PrinterEnum::DuplexMode::DuplexLongSide);
        m_instance->setDefaultDuplexMode(PrinterEnum::DuplexMode::DuplexNone);
        QCOMPARE(m_instance->defaultDuplexMode(), PrinterEnum::DuplexMode::DuplexNone);
        m_instance->setDefaultDuplexMode(PrinterEnum::DuplexMode::DuplexShortSide);
        QCOMPARE(m_instance->defaultDuplexMode(), PrinterEnum::DuplexMode::DuplexShortSide);
    }
    void testSetDefaultDuplexMode()
    {
        // Add support
        QList<PrinterEnum::DuplexMode> modes({PrinterEnum::DuplexMode::DuplexNone, PrinterEnum::DuplexMode::DuplexLongSide});
        m_backend->m_supportedDuplexModes = modes;

        m_instance->setDefaultDuplexMode(PrinterEnum::DuplexMode::DuplexLongSide);

        QVariant duplexVar = m_backend->printerOptions[m_printerName].value("Duplex");
        QStringList duplexVals = duplexVar.toStringList();
         QCOMPARE(
            duplexVals.at(0),
            (QString) Utils::duplexModeToPpdChoice(PrinterEnum::DuplexMode::DuplexLongSide)
        );
    }
    void testSetDefaultPageSize_data()
    {
        QTest::addColumn<QList<QPageSize>>("sizes");
        QTest::addColumn<QPageSize>("size");
        QTest::addColumn<bool>("expectCupsCommunication");
        QTest::addColumn<QString>("expectedValue");
        {
            QList<QPageSize> sizes; // none supported
            QPageSize size(QPageSize::A4);
            QTest::newRow("unsupported") << sizes << size << false << "";
        }
        {
            QList<QPageSize> sizes({QPageSize(QPageSize::A4)});
            QPageSize size(QPageSize::A4);
            QTest::newRow("supported") << sizes << size << true << "A4";
        }
        {
            QPageSize custom(QSize(100, 100), "foo");
            QList<QPageSize> sizes({custom});
            QTest::newRow("supported, but non-ppd size") << sizes << custom << false << "";
        }
    }
    void testDefaultPageSize()
    {
        auto targetSize = QPageSize(QPageSize::A4);
        m_backend->m_defaultPageSize = targetSize;
        QCOMPARE(m_instance->defaultPageSize(), targetSize);
    }
    void testSupportedPageSizes_data()
    {
        auto supported = QList<QPageSize>({QPageSize(QPageSize::A4), QPageSize(QPageSize::Letter)});
        m_backend->m_supportedPageSizes = supported;
        QCOMPARE(m_instance->supportedPageSizes(), supported);
    }
    void testSetDefaultPageSize()
    {
        QFETCH(QList<QPageSize>, sizes);
        QFETCH(QPageSize, size);
        QFETCH(bool, expectCupsCommunication);
        QFETCH(QString, expectedValue);
        m_backend->m_supportedPageSizes = sizes;

        m_instance->setDefaultPageSize(size);

        QVariant pageSizeVar = m_backend->printerOptions[m_printerName].value("PageSize");
        QStringList pageSizeVals = pageSizeVar.toStringList();

        if (expectCupsCommunication) {
            QCOMPARE(pageSizeVals.at(0), expectedValue);
        }
    }
    void testDefaultColorMode()
    {
        ColorModel colorModel;
        QCOMPARE(m_instance->defaultColorModel(), colorModel);
    }
    void testSupportedColorModels()
    {
        ColorModel a;
        a.text = "Gray";

        ColorModel b;
        b.text = "RBG";
        QList<ColorModel> models({a, b});

        PrinterBackend *backend = new MockPrinterBackend(m_printerName);
        ((MockPrinterBackend*) backend)->printerOptions[m_printerName].insert(
            "SupportedColorModels", QVariant::fromValue(models));
        Printer p(backend);
        QCOMPARE(p.supportedColorModels(), models);
    }
    void testDefaultPrintQuality()
    {
        PrintQuality quality;
        QCOMPARE(m_instance->defaultPrintQuality(), quality);
    }
    void testSupportedQualities()
    {
        PrintQuality a;
        a.name = "Poor";

        PrintQuality b;
        b.name = "Worse";
        QList<PrintQuality> qualities({a, b});

        MockPrinterBackend *backend = new MockPrinterBackend(m_printerName);
        backend->printerOptions[m_printerName].insert(
            "SupportedPrintQualities", QVariant::fromValue(qualities));
        Printer p(backend);
        QCOMPARE(p.supportedPrintQualities(), qualities);
    }
    void testPdfPrinter()
    {
        PrinterBackend *backend = new PrinterPdfBackend(m_printerName);
        Printer p(backend);
        QCOMPARE(p.defaultColorModel().name, QString("RGB"));
        QCOMPARE(p.defaultPageSize(), QPageSize(QPageSize::A4));
        QCOMPARE(p.defaultDuplexMode(), PrinterEnum::DuplexMode::DuplexNone);
        QCOMPARE(p.type(), PrinterEnum::PrinterType::PdfType);
    }
    void testState()
    {
        m_backend->m_state = PrinterEnum::State::AbortedState;
        QCOMPARE(m_instance->state(), m_backend->m_state);
    }
    void testEnabled()
    {
        m_backend->m_state = PrinterEnum::State::ErrorState;
        QCOMPARE(m_instance->enabled(), false);
        m_backend->m_state = PrinterEnum::State::IdleState;
        QCOMPARE(m_instance->enabled(), true);

    }
    void testSetEnabled()
    {
        /* This is needed to make a printer disabled. If it's not disabled,
        the printer does not attempt to set a new value. */
        m_backend->m_state = PrinterEnum::State::ErrorState;

        m_instance->setEnabled(true);
        QCOMPARE(m_backend->enableds[m_printerName], true);

    }
    void testAcceptJobs()
    {
        MockPrinterBackend *backend = new MockPrinterBackend(m_printerName);

        backend->printerOptions[m_printerName].insert("AcceptJobs", false);
        Printer p(backend);
        QVERIFY(!p.acceptJobs());

        backend->printerOptions[m_printerName].insert("AcceptJobs", true);
        Printer p2(backend);
        QVERIFY(p2.acceptJobs());

    }
    void testSetAcceptJobs()
    {
        MockPrinterBackend *backend = new MockPrinterBackend(m_printerName);

        backend->printerOptions[m_printerName].insert("AcceptJobs", false);
        Printer p(backend);
        p.setAcceptJobs(true);
        QVERIFY(backend->printerOptions[m_printerName]["AcceptJobs"].toBool());

        backend->printerOptions[m_printerName].insert("AcceptJobs", true);
        Printer p2(backend);
        p2.setAcceptJobs(false);
        QVERIFY(!backend->printerOptions[m_printerName]["AcceptJobs"].toBool());
    }
    void testShared()
    {
        MockPrinterBackend *backend = new MockPrinterBackend(m_printerName);

        backend->printerOptions[m_printerName].insert("Shared", false);
        Printer p(backend);
        QVERIFY(!p.shared());

        backend->printerOptions[m_printerName].insert("Shared", true);
        Printer p2(backend);
        QVERIFY(p2.shared());
    }
    void testSetShared()
    {
        MockPrinterBackend *backend = new MockPrinterBackend(m_printerName);

        backend->printerOptions[m_printerName].insert("Shared", false);
        Printer p(backend);
        p.setShared(true);
        QVERIFY(backend->printerOptions[m_printerName]["Shared"].toBool());

        backend->printerOptions[m_printerName].insert("Shared", true);
        Printer p2(backend);
        p2.setShared(false);
        QVERIFY(!backend->printerOptions[m_printerName]["Shared"].toBool());
    }
    void testJobs()
    {
        JobModel jobs;
        jobs.setObjectName("testjobs");
        m_instance->setJobModel(&jobs);

        QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel *>(
            m_instance->jobs()
        );
        QCOMPARE(proxy->sourceModel()->objectName(), jobs.objectName());
    }

    /* There was a bug causing QML thread assertion to fail. The assertion
    requires all QObjects accessed from QML to be in the same thread as QML.
    For newly loaded non-proxy printers, this was not the case. This test
    serves as a regression test, as well as a place where we can make sure
    every QObject of the Printer API moves thread when the printer itself
    moves.

    For POD, this is inconsequential. */
    void testPrinterMovesProperlyFromThread()
    {
        MockPrinterBackend *backend = new MockPrinterBackend(m_printerName);

        Printer p(backend);
        QThread thread;
        p.moveToThread(&thread);

        QCOMPARE(p.thread(), &thread);

        // Ideally, all QObjects in the Printer API needs to be tested here.
        QCOMPARE(p.jobs()->thread(), &thread);
    }

    void testUpdateFrom()
    {
        // Setup any variables for flipped values
        bool newAcceptJobs = !m_instance->acceptJobs();
        int newCopies = m_instance->copies() + 1;
        QString newDeviceUri = m_instance->deviceUri() + "/test";
        bool newShared = !m_instance->shared();
        QString newLastMessage = m_instance->lastMessage() + "test";

        // Setup color model and quality models
        ColorModel colorA;
        colorA.text = "ColorA";

        ColorModel defaultColorModel;
        defaultColorModel.text = "ColorB";
        QList<ColorModel> colorModels({colorA, defaultColorModel});

        PrintQuality qualityA;
        qualityA.name = "QualityA";

        PrintQuality defaultPrintQuality;
        defaultPrintQuality.name = "QualityB";
        QList<PrintQuality> qualities({qualityA, defaultPrintQuality});

        QList<PrinterEnum::DuplexMode> duplexModes({
            PrinterEnum::DuplexMode::DuplexNone,
            PrinterEnum::DuplexMode::DuplexLongSide,
            PrinterEnum::DuplexMode::DuplexShortSide,
            PrinterEnum::DuplexMode::DuplexShortSide
        });

        auto newDefaultDuplexMode = m_instance->defaultDuplexMode() == PrinterEnum::DuplexMode::DuplexNone
                ? PrinterEnum::DuplexMode::DuplexShortSide
                : PrinterEnum::DuplexMode::DuplexNone;

        // Create a printer that has different settings to the default
        QString printerName = "test-printer-b";

        MockPrinterBackend *backend = new MockPrinterBackend(printerName);
        backend->printerOptions[printerName].insert("AcceptJobs", newAcceptJobs);
        backend->printerOptions[printerName].insert("Copies", QString::number(newCopies));
        backend->printerOptions[printerName].insert("DefaultColorModel", QVariant::fromValue(defaultColorModel));
        backend->printerOptions[printerName].insert("DefaultPrintQuality", QVariant::fromValue(defaultPrintQuality));
        backend->printerOptions[printerName].insert(
            "DeviceUri", newDeviceUri);
        backend->printerOptions[printerName].insert("Shared", newShared);
        backend->printerOptions[printerName].insert(
            "StateMessage", newLastMessage);
        backend->printerOptions[printerName].insert(
            "SupportedColorModels", QVariant::fromValue(colorModels));
        backend->printerOptions[printerName].insert(
            "SupportedPrintQualities", QVariant::fromValue(qualities));

        backend->m_supportedDuplexModes = duplexModes;
        backend->printerOptions[printerName].insert(
            "Duplex", Utils::duplexModeToPpdChoice(newDefaultDuplexMode));

        QSharedPointer<Printer> p = QSharedPointer<Printer>(new Printer(backend));

        qDebug() << p->deviceUri();

        // Update the default printer from this
        m_instance->updateFrom(p);

        // Check we have the new values
        QCOMPARE(m_instance->acceptJobs(), newAcceptJobs);
        QCOMPARE(m_instance->copies(), newCopies);
        QCOMPARE(m_instance->defaultColorModel(), defaultColorModel);
        QCOMPARE(m_instance->defaultPrintQuality(), defaultPrintQuality);
        QCOMPARE(m_instance->deviceUri(), newDeviceUri);
        QCOMPARE(m_instance->shared(), newShared);
        QCOMPARE(m_instance->lastMessage(), newLastMessage);
        QCOMPARE(m_instance->supportedColorModels(), colorModels);
        QCOMPARE(m_instance->supportedPrintQualities(), qualities);

        // We test that the duplexModes have changed to check that the backend
        // has changed, as m_backend is private we can't check directly
        QCOMPARE(m_instance->defaultDuplexMode(), newDefaultDuplexMode);
        QCOMPARE(m_instance->supportedDuplexModes(), duplexModes);
    }

private:
    QString m_printerName = "my-printer";
    MockPrinterBackend *m_backend = nullptr;
    QSharedPointer<Printer> m_instance;
};

QTEST_GUILESS_MAIN(TestPrinter)
#include "tst_printer.moc"
