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

#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

class TestJobModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_backend = new MockPrinterBackend;
        m_model = new JobModel(m_backend);
    }
    void cleanup()
    {
        QSignalSpy destroyedSpy(m_model, SIGNAL(destroyed(QObject*)));
        m_model->deleteLater();
        QTRY_COMPARE(destroyedSpy.count(), 1);
        delete m_backend;
    }
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
        m_backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);
        QCOMPARE(removeSpy.count(), 1);

        // Check item was removed
        QList<QVariant> args = removeSpy.at(0);
        QCOMPARE(args.at(1).toInt(), 0);
        QCOMPARE(args.at(2).toInt(), 0);
    }
    void testMove()
    {
        // Add two jobs.
        auto job1 = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        auto job2 = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 2));
        m_backend->m_jobs << job1 << job2;
        m_backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);

        m_backend->m_jobs.move(0, 1);
        // Triggers a move.
        QSignalSpy moveSpy(m_model, SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)));
        m_backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);
        QCOMPARE(moveSpy.count(), 1);
        QList<QVariant> args = moveSpy.at(0);
        QCOMPARE(args.at(1).toInt(), 1);
        QCOMPARE(args.at(2).toInt(), 1);
        QCOMPARE(args.at(4).toInt(), 0);
    }
    void testModify()
    {
        auto jobBefore = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));

        m_backend->m_jobs << jobBefore;
        m_backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);

        auto jobAfter = QSharedPointer<PrinterJob>(new PrinterJob("test-printer", m_backend, 1));
        jobAfter->setCopies(100);
        m_backend->m_jobs.replace(0, jobAfter);

        // Triggers a change.
        QSignalSpy changedSpy(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));
        m_backend->mockJobCreated("", "", "", 1, "", true, 100, 1, "", "", 1);
        QCOMPARE(changedSpy.count(), 1);
    }

private:
    MockPrinterBackend *m_backend;
    JobModel *m_model;
};

QTEST_GUILESS_MAIN(TestJobModel)
#include "tst_jobmodel.moc"
