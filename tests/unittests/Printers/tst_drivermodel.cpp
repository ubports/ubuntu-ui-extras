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
#include "models/drivermodel.h"

#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

class TestDriverModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_backend = new MockPrinterBackend;
        m_model = new DriverModel(m_backend);
    }
    void cleanup()
    {
        QSignalSpy destroyedSpy(m_model, SIGNAL(destroyed(QObject*)));
        m_model->deleteLater();
        QTRY_COMPARE(destroyedSpy.count(), 1);
        delete m_backend;
    }
    void testDrivers_data()
    {
        QTest::addColumn<QList<PrinterDriver>>("drivers");
        QTest::addColumn<int>("expectedCount");
        {
            QList<PrinterDriver> drivers;
            QTest::newRow("none") << drivers << 0;
        }
        {
            QList<PrinterDriver> drivers({PrinterDriver(), PrinterDriver()});
            QTest::newRow("some") << drivers << 2;
        }
    }
    void testDrivers()
    {
        QFETCH(QList<PrinterDriver>, drivers);
        QFETCH(int, expectedCount);

        m_model->load();
        getBackend()->mockDriversLoaded(drivers);
        QCOMPARE(m_model->rowCount(), expectedCount);
    }
    void testFiltering_data()
    {
        QTest::addColumn<QList<PrinterDriver>>("drivers");
        QTest::addColumn<QList<PrinterDriver>>("expectedDrivers");
        QTest::addColumn<QString>("filter");

        {
            QList<PrinterDriver> drivers;
            QList<PrinterDriver> expectedDrivers;

            PrinterDriver canon;
            canon.makeModel = "Canon Foojet";

            PrinterDriver hp;
            hp.makeModel = "HP Laserfjert";

            drivers << canon << hp;
            expectedDrivers << hp;

            QTest::newRow("filter hp") << drivers << expectedDrivers << "hp";
        }
        {
            QList<PrinterDriver> drivers;
            QList<PrinterDriver> expectedDrivers;

            PrinterDriver canon;
            canon.makeModel = "Canon 4500 Foojet";

            PrinterDriver canon2;
            canon2.makeModel = "rabble rabble canon 4500 masterjet";

            PrinterDriver hp;
            hp.makeModel = "HP Laserfjert";

            drivers << canon << canon2 << hp;
            expectedDrivers << canon << canon2;

            QTest::newRow("filter canon 4500 printers") << drivers << expectedDrivers << "canon 4500";
        }
    }
    void testFiltering()
    {
        QFETCH(QList<PrinterDriver>, drivers);
        QFETCH(QList<PrinterDriver>, expectedDrivers);
        QFETCH(QString, filter);

        m_model->load();
        getBackend()->mockDriversLoaded(drivers);

        QSignalSpy filterCompleteSpy(m_model, SIGNAL(filterComplete()));
        m_model->setFilter(filter);
        QTRY_COMPARE(filterCompleteSpy.count(), 1);

        QCOMPARE(m_model->rowCount(), expectedDrivers.size());
        for (int i = 0; i < m_model->rowCount(); i++) {
            QCOMPARE(
                expectedDrivers.at(i).makeModel,
                m_model->data(m_model->index(i), DriverModel::Roles::MakeModelRole).toByteArray()
            );
        }
    }
private:
    PrinterBackend *m_backend;
    DriverModel *m_model;
    MockPrinterBackend* getBackend()
    {
        return (MockPrinterBackend*) m_backend;
    }
};

QTEST_GUILESS_MAIN(TestDriverModel)
#include "tst_drivermodel.moc"
