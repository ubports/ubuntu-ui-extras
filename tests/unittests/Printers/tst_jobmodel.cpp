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
#include "models/jobmodel.h"
#include "printers/printers.h"

#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>


class MockPrinters : QObject
{
    Q_OBJECT
public:
    MockPrinters(PrinterBackend *backend, JobModel *model, QObject* parent=Q_NULLPTR)
        : QObject(parent)
        , m_backend(backend)
        , m_jobs(model)
    {
        connect(m_jobs, &QAbstractItemModel::rowsInserted, [this](
                const QModelIndex &parent, int first, int) {
            int jobId = m_jobs->data(m_jobs->index(first, 0, parent),
                                    JobModel::Roles::IdRole).toInt();
            QString printerName = m_jobs->data(
                m_jobs->index(first, 0, parent),
                JobModel::Roles::PrinterNameRole
            ).toString();

            jobAdded(m_jobs->getJob(printerName, jobId));
        });
    }
    QList<QSharedPointer<Printer>> m_printers;
public Q_SLOTS:
    void jobAdded(QSharedPointer<PrinterJob> job)
    {
        Q_FOREACH(auto printer, m_printers) {
            if (printer->name() == job->printerName()) {
                m_backend->requestJobExtendedAttributes(printer, job);
            }
        }
    }

private:
    PrinterBackend *m_backend;
    JobModel *m_jobs;
};


class TestJobModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_backend = new MockPrinterBackend;
        m_model = new JobModel(m_backend);

        // Setup Printers otherwise job's aren't loaded
        m_printers = new MockPrinters(m_backend, m_model);

        PrinterBackend* backend = new MockPrinterBackend("test-printer");
        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);
        m_printers->m_printers << printer;
    }
    void cleanup()
    {
        QSignalSpy destroyedSpy(m_model, SIGNAL(destroyed(QObject*)));
        m_model->deleteLater();
        QTRY_COMPARE(destroyedSpy.count(), 1);
        delete m_backend;
    }

    // Tests for adding/removing/changing things in the model

    void testInsert()
    {
        QSignalSpy countSpy(m_model, SIGNAL(countChanged()));
        QSignalSpy insertSpy(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)));

        auto job = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend));
        m_backend->m_jobs << job;

        // Trigger update.
        m_backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);

        QCOMPARE(m_model->count(), 1);
        QCOMPARE(countSpy.count(), 1);
        QCOMPARE(insertSpy.count(), 1);
    }
    void testRemove()
    {
        // Add one.
        auto job = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend));
        m_backend->m_jobs << job;
        m_backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);

        QCOMPARE(m_model->count(), 1);

        /* Trigger another update, ignore the signal name here; there's a
        catchall handler in the model. */
        QSignalSpy removeSpy(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)));
        m_backend->m_jobs.clear();
        m_backend->mockJobCompleted("", "", "", 1, "", true, 100, 1, "", "", 1);
        QCOMPARE(removeSpy.count(), 1);

        // Check item was removed
        QList<QVariant> args = removeSpy.at(0);
        QCOMPARE(args.at(1).toInt(), 0);
        QCOMPARE(args.at(2).toInt(), 0);
    }
    void testModify()
    {
        auto jobBefore = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        int impressions_count = 1;

        m_backend->m_jobs << jobBefore;
        m_backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", impressions_count);

        // Triggers a change.
        QSignalSpy changedSpy(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));

        impressions_count = 5;
        m_backend->mockJobState("", "", "", 1, "", true, 100, 1, "", "", impressions_count);

        QCOMPARE(changedSpy.count(), 1);
    }

    // Tests for the roles in the model exposed to QML

    void testIdRole()
    {
        auto job = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 100));
        m_backend->m_jobs << job;
        m_backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 1);
        QCOMPARE(job->jobId(), 100);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::IdRole).toInt(), 100);
    }
    void testCollateRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setCollate(false);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setCollate(true);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);

        QCOMPARE(m_model->data(m_model->index(0), JobModel::CollateRole).toBool(), false);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::CollateRole).toBool(), true);
    }
    void testColorModelRole()
    {
        // FIXME: read comment in JobModel::updateJob
        QSKIP("We are ignoring colorModel for now as it requires a loaded Printer for the PrinterJob.");

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

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);
        m_printers->m_printers << printer;

        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("a-printer", backend, 1));
        jobA->setPrinter(printer);
        jobA->loadDefaults();
        jobA->setColorModel(models.indexOf(a));

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("a-printer", backend, 2));
        jobB->setPrinter(printer);
        jobB->loadDefaults();
        jobB->setColorModel(models.indexOf(b));

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "a-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "a-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::ColorModelRole).toString(),
                 a.text);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::ColorModelRole).toString(),
                 b.name);
    }
    void testCompletedTimeRole()
    {
        QDateTime dateTimeA = QDateTime::currentDateTime();
        QDateTime dateTimeB = QDateTime::currentDateTime().addSecs(100);

        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setCompletedTime(dateTimeA);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setCompletedTime(dateTimeB);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::CompletedTimeRole).toDateTime(),
                 dateTimeA);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::CompletedTimeRole).toDateTime(),
                 dateTimeB);
    }
    void testCopiesRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setCopies(2);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setCopies(5);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::CopiesRole).toInt(), 2);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::CopiesRole).toInt(), 5);
    }
    void testCreationTimeRole()
    {
        QDateTime dateTimeA = QDateTime::currentDateTime();
        QDateTime dateTimeB = QDateTime::currentDateTime().addSecs(100);

        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setCreationTime(dateTimeA);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setCreationTime(dateTimeB);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::CreationTimeRole).toDateTime(),
                 dateTimeA);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::CreationTimeRole).toDateTime(),
                 dateTimeB);
    }
    void testDuplexRole()
    {
        // FIXME: read comment in JobModel::updateJob
        QSKIP("We are ignoring duplex for now as it requires a loaded Printer for the PrinterJob.");

        QList<PrinterEnum::DuplexMode> modes({
            PrinterEnum::DuplexMode::DuplexNone,
            PrinterEnum::DuplexMode::DuplexLongSide,
            PrinterEnum::DuplexMode::DuplexShortSide,
        });

        ((MockPrinterBackend*) m_backend)->m_supportedDuplexModes = modes;

        PrinterBackend* backend = new MockPrinterBackend("test-printer");
        ((MockPrinterBackend*) backend)->m_supportedDuplexModes = modes;

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", backend, 1));
        jobA->setPrinter(printer);
        jobA->setDuplexMode(modes.indexOf(PrinterEnum::DuplexMode::DuplexLongSide));

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", backend, 2));
        jobB->setPrinter(printer);
        jobB->setDuplexMode(modes.indexOf(PrinterEnum::DuplexMode::DuplexNone));

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::DuplexRole).toString(),
                 Utils::duplexModeToUIString(PrinterEnum::DuplexMode::DuplexLongSide));
        QCOMPARE(m_model->data(m_model->index(1), JobModel::DuplexRole).toString(),
                 Utils::duplexModeToUIString(PrinterEnum::DuplexMode::DuplexNone));
    }
    void testHeldRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setState(PrinterEnum::JobState::Pending);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setState(PrinterEnum::JobState::Held);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::HeldRole).toBool(), false);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::HeldRole).toBool(), true);
    }
    void testImpressionsCompletedRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setImpressionsCompleted(2);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setImpressionsCompleted(5);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::ImpressionsCompletedRole).toInt(), 2);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::ImpressionsCompletedRole).toInt(), 5);
    }
    void testLandscapeRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setLandscape(false);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setLandscape(true);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::LandscapeRole).toBool(), false);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::LandscapeRole).toBool(), true);
    }
    void testMessagesRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setMessages(QStringList() << "a-message" << "b-message");

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setMessages(QStringList() << "c-message" << "d-message");

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::MessagesRole).toStringList(),
                 QStringList() << "a-message" << "b-message");
        QCOMPARE(m_model->data(m_model->index(1), JobModel::MessagesRole).toStringList(),
                 QStringList() << "c-message" << "d-message");
    }
    void testPrinterNameRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("a-printer", m_backend, 1));

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("b-printer", m_backend, 2));

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "a-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "b-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::PrinterNameRole).toString(), QString("a-printer"));
        QCOMPARE(m_model->data(m_model->index(1), JobModel::PrinterNameRole).toString(), QString("b-printer"));
    }
    void testPrintRangeRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setPrintRange("1-3,5");

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setPrintRange("-3,6,10-");

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::PrintRangeRole).toString(), QString("1-3,5"));
        QCOMPARE(m_model->data(m_model->index(1), JobModel::PrintRangeRole).toString(), QString("-3,6,10-"));
    }
    void testPrintRangeModeRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setPrintRangeMode(PrinterEnum::PrintRange::AllPages);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setPrintRangeMode(PrinterEnum::PrintRange::PageRange);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::PrintRangeModeRole).value<PrinterEnum::PrintRange>(), PrinterEnum::PrintRange::AllPages);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::PrintRangeModeRole).value<PrinterEnum::PrintRange>(), PrinterEnum::PrintRange::PageRange);
    }
    void testProcessingTimeRole()
    {
        QDateTime dateTimeA = QDateTime::currentDateTime();
        QDateTime dateTimeB = QDateTime::currentDateTime().addSecs(100);

        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setProcessingTime(dateTimeA);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setProcessingTime(dateTimeB);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::ProcessingTimeRole).toDateTime(),
                 dateTimeA);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::ProcessingTimeRole).toDateTime(),
                 dateTimeB);
    }
    void testQualityRole()
    {
        // FIXME: read comment in JobModel::updateJob
        QSKIP("We are ignoring quality for now as it requires a loaded Printer for the PrinterJob.");

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

        auto printer = QSharedPointer<Printer>(new Printer(backend));
        m_backend->mockPrinterLoaded(printer);

        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("a-printer", backend, 1));
        jobA->setPrinter(printer);
        jobA->setQuality(qualities.indexOf(a));

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("a-printer", backend, 2));
        jobB->setPrinter(printer);
        jobB->setQuality(qualities.indexOf(b));

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "a-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "a-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::QualityRole).toString(),
                 a.text);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::QualityRole).toString(),
                 b.name);
    }
    void testReverseRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setReverse(false);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setReverse(true);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::ReverseRole).toBool(), false);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::ReverseRole).toBool(), true);
    }
    void testSizeRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setSize(32);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setSize(64);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::SizeRole).toInt(), 32);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::SizeRole).toInt(), 64);
    }
    void testStateRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setState(PrinterEnum::JobState::Pending);

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setState(PrinterEnum::JobState::Processing);

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::StateRole).value<PrinterEnum::JobState>(), PrinterEnum::JobState::Pending);
        QCOMPARE(m_model->data(m_model->index(1), JobModel::StateRole).value<PrinterEnum::JobState>(), PrinterEnum::JobState::Processing);
    }
    void testTitleRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setTitle("a-job");

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setTitle("b-job");

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::TitleRole).toString(), QString("a-job"));
        QCOMPARE(m_model->data(m_model->index(1), JobModel::TitleRole).toString(), QString("b-job"));
    }
    void testUserRole()
    {
        auto jobA = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobA->setUser("a-user");

        auto jobB = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        jobB->setUser("b-user");

        m_backend->m_jobs << jobA << jobB;
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 1, 1, "", "", 1);
        m_backend->mockJobCreated("", "", "test-printer", 1, "", true, 2, 1, "", "", 1);

        QTRY_COMPARE(m_model->count(), 2);
        QCOMPARE(m_model->data(m_model->index(0), JobModel::UserRole).toString(), QString("a-user"));
        QCOMPARE(m_model->data(m_model->index(1), JobModel::UserRole).toString(), QString("b-user"));
    }

private:
    MockPrinterBackend *m_backend;
    JobModel *m_model;
    MockPrinters *m_printers;
};

QTEST_GUILESS_MAIN(TestJobModel)
#include "tst_jobmodel.moc"
