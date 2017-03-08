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
#include "models/devicemodel.h"
#include "structs.h"

#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(DeviceModel::Roles)

class TestDeviceModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_backend = new MockPrinterBackend();
        m_model = new DeviceModel(m_backend);
    }
    void cleanup()
    {
        QSignalSpy destroyedSpy(m_model, SIGNAL(destroyed(QObject*)));
        m_model->deleteLater();
        QTRY_COMPARE(destroyedSpy.count(), 1);
        delete m_backend;
    }
    void testAcceptedDevices_data()
    {
        QTest::addColumn<Device>("device");
        QTest::addColumn<bool>("accepted");

        {
            Device d; d.uri = "dnssd";
            QTest::newRow("empty uri") << d << false;
        }
        {
            Device d; d.uri = "dnssd://foo-bar";
            QTest::newRow("non-empty uri") << d << true;
        }
        {
            Device d; d.uri = "dnssd:";
            QTest::newRow("non-empty uri, malformed") << d << false;
        }
    }
    void testAcceptedDevices()
    {
        QFETCH(Device, device);
        QFETCH(bool, accepted);
        m_backend->mockDeviceFound(device);

        QCOMPARE(m_model->rowCount(), accepted ? 1 : 0);
    }
    void testInsert()
    {
        QSignalSpy countSpy(m_model, SIGNAL(countChanged()));
        QSignalSpy insertSpy(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)));

        Device d; d.uri = "dnssd://foo-bar";
        m_backend->mockDeviceFound(d);

        QCOMPARE(m_model->count(), 1);
        QCOMPARE(countSpy.count(), 1);
        QCOMPARE(insertSpy.count(), 1);
    }
    void testDuplicatesIgnored()
    {
        QSignalSpy countSpy(m_model, SIGNAL(countChanged()));
        QSignalSpy insertSpy(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)));

        Device d; d.uri = "dnssd://foo-bar";
        m_backend->mockDeviceFound(d);

        Device d1; d1.uri = "dnssd://foo-bar";
        m_backend->mockDeviceFound(d1);

        QCOMPARE(m_model->count(), 1);
        QCOMPARE(countSpy.count(), 1);
        QCOMPARE(insertSpy.count(), 1);
    }
    void testRoles_data()
    {
        QTest::addColumn<DeviceModel::Roles>("role");
        QTest::addColumn<QVariant>("value");
        QTest::addColumn<Device>("device");

        {
            Device d; d.uri = "ipp://foo/bar";
            d.id = "foo";
            DeviceModel::Roles role(DeviceModel::IdRole);
            QVariant value(d.id);
            QTest::newRow("DisplayRole") << role << value << d;
        }
        {
            Device d; d.uri = "ipp://foo/bar";
            d.info = "foo";
            DeviceModel::Roles role(DeviceModel::InfoRole);
            QVariant value(d.info);
            QTest::newRow("InfoRole") << role << value << d;
        }
        {
            Device d; d.uri = "ipp://foo/bar";
            DeviceModel::Roles role(DeviceModel::UriRole);
            QVariant value(d.uri);
            QTest::newRow("UriRole") << role << value << d;
        }
        {
            Device d; d.uri = "ipp://foo/bar";
            d.location = "home";
            DeviceModel::Roles role(DeviceModel::LocationRole);
            QVariant value(d.location);
            QTest::newRow("LocationRole") << role << value << d;
        }
        {
            Device d; d.uri = "ipp://foo/bar";
            d.makeModel = "hp";
            DeviceModel::Roles role(DeviceModel::MakeModelRole);
            QVariant value(d.makeModel);
            QTest::newRow("MakeModelRole") << role << value << d;
        }
    }
    void testRoles()
    {
        QFETCH(DeviceModel::Roles, role);
        QFETCH(QVariant, value);
        QFETCH(Device, device);

        m_backend->mockDeviceFound(device);
        QCOMPARE(m_model->data(m_model->index(0), role), value);
    }
private:
    MockPrinterBackend *m_backend;
    DeviceModel *m_model;
};

QTEST_GUILESS_MAIN(TestDeviceModel)
#include "tst_printerdevicemodel.moc"

