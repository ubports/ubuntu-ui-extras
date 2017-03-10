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

#include "enums.h"
#include "structs.h"

#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(PrinterEnum::DeviceType)

class TestDevice : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testUriToType_data()
    {
        QTest::addColumn<Device>("device");
        QTest::addColumn<PrinterEnum::DeviceType>("expected");

        {
            Device d;
            QTest::newRow("UnknownType") << d << PrinterEnum::DeviceType::UnknownType;
        }
        {
            Device d; d.uri = "dnssd";
            QTest::newRow("DNSSDType") << d << PrinterEnum::DeviceType::DNSSDType;
        }
        {
            Device d; d.uri = "lpd";
            QTest::newRow("LPDType") << d << PrinterEnum::DeviceType::LPDType;
        }
        {
            Device d; d.uri = "ipps";
            QTest::newRow("IppSType") << d << PrinterEnum::DeviceType::IppSType;
        }
        {
            Device d; d.uri = "ipp14";
            QTest::newRow("Ipp14Type") << d << PrinterEnum::DeviceType::Ipp14Type;
        }
        {
            Device d; d.uri = "http";
            QTest::newRow("HttpType") << d << PrinterEnum::DeviceType::HttpType;
        }
        {
            Device d; d.uri = "beh";
            QTest::newRow("BehType") << d << PrinterEnum::DeviceType::BehType;
        }
        {
            Device d; d.uri = "socket";
            QTest::newRow("SocketType") << d << PrinterEnum::DeviceType::SocketType;
        }
        {
            Device d; d.uri = "https";
            QTest::newRow("HttpsType") << d << PrinterEnum::DeviceType::HttpsType;
        }
        {
            Device d; d.uri = "ipp";
            QTest::newRow("IppType") << d << PrinterEnum::DeviceType::IppType;
        }
        {
            Device d; d.uri = "hp";
            QTest::newRow("HPType") << d << PrinterEnum::DeviceType::HPType;
        }
        {
            Device d; d.uri = "usb";
            QTest::newRow("USBType") << d << PrinterEnum::DeviceType::USBType;
        }
        {
            Device d; d.uri = "hpfax";
            QTest::newRow("HPFaxType") << d << PrinterEnum::DeviceType::HPFaxType;
        }
    }
    void testUriToType()
    {
        QFETCH(Device, device);
        QFETCH(PrinterEnum::DeviceType, expected);

        QCOMPARE(device.type(), expected);
    }
    void testToString_data()
    {
        QTest::addColumn<QString>("id");
        QTest::addColumn<QString>("expected");

        {
            Device d;
            QTest::newRow("a hp printer")
                << "MFG:HP;MDL:Color LaserJet 4500CMD:PDF,PS,JPEG,PNG,PWG,URF"
                << "HP Color LaserJet 4500CMD";
        }
        {
            Device d; ;
            QTest::newRow("Andrew's hp printer")
                << "MFG:HP;MDL:Officejet 5740 series;CMD:PCL,JPEG,URF,PWG;"
                << "HP Officejet 5740 series";
        }
        {
            Device d; ;
            QTest::newRow("Base case with A as manufacturer, B as model.")
                << "MFG:A;MDL:B"
                << "A B";
        }
    }
    void testToString()
    {
        QFETCH(QString, id);
        QFETCH(QString, expected);

        Device d; d.id = id;
        QCOMPARE(d.toString(), expected);
    }
};

QTEST_GUILESS_MAIN(TestDevice)
#include "tst_printerdevice.moc"
