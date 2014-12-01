/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "photo-image-provider.h"

#include <QTest>
#include <QSignalSpy>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

class PhotoEditorPhotoImageProviderTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testEmptyOrInvalid();
    void testRotation();
    void testCache();

    void cleanupTestCase();

private:        
    PhotoImageProvider *m_provider;
    QTemporaryDir m_workingDir;
};

void PhotoEditorPhotoImageProviderTest::initTestCase()
{
    m_provider = new PhotoImageProvider();

    QDir rc = QDir(":/assets/");
    QDir dest = QDir(m_workingDir.path());
    Q_FOREACH(const QString &name, rc.entryList())
    {
        QFile::copy(rc.absoluteFilePath(name), dest.absoluteFilePath(name));
        QFile::setPermissions(dest.absoluteFilePath(name),
                              QFile::WriteOwner | QFile::ReadOwner);
    }
}

void PhotoEditorPhotoImageProviderTest::cleanupTestCase()
{
    // temporary dir will be deleted when m_workingDir goes out of scope
    delete m_provider;
}

void PhotoEditorPhotoImageProviderTest::testRotation()
{
    // Loading an image without rotation is equivalent to loading it directly
    QDir source = QDir(m_workingDir.path());
    QImage image = m_provider->requestImage(source.absoluteFilePath("windmill.jpg"), 0, QSize());
    QImage direct(source.absoluteFilePath("windmill.jpg"));

    QVERIFY(!image.isNull());
    QVERIFY(image == direct);

    // Loading an image with EXIF rotation will rotate it
    image = m_provider->requestImage(source.absoluteFilePath("windmill_rotated_90.jpg"), 0, QSize());
    direct = QImage(source.absoluteFilePath("windmill.jpg"));

    QVERIFY(!image.isNull());
    QVERIFY(image != direct);

    QTransform t;
    t.rotate(90);
    QImage rotated = direct.transformed(t);
    QVERIFY(image == rotated);
}

void PhotoEditorPhotoImageProviderTest::testCache()
{
    // Work on a copy to avoid disturbing other tests
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("testcache.jpg");
    QFile::copy(source.absoluteFilePath("windmill.jpg"), path);

    // First request an image so that it gets cached
    QImage image = m_provider->requestImage(path, 0, QSize());
    QVERIFY(!image.isNull());
    QVERIFY(image.width() == 400);
    QVERIFY(image.height() == 267);

    // Copy another file with different rotation into the same file
    QFile main(path);
    main.open(QIODevice::WriteOnly);

    QFile other(source.absoluteFilePath("windmill_rotated_90.jpg"));
    other.open(QIODevice::ReadOnly);

    main.write(other.readAll());
    main.close();
    other.close();

    image = m_provider->requestImage(path, 0, QSize());
    QVERIFY(image.width() == 267);
    QVERIFY(image.height() == 400);
}

void PhotoEditorPhotoImageProviderTest::testEmptyOrInvalid()
{
    QImage image = m_provider->requestImage("", 0, QSize());
    QVERIFY(image.isNull());

    image = m_provider->requestImage("do-not-exist.jpg", 0, QSize());
    QVERIFY(image.isNull());
}

QTEST_MAIN(PhotoEditorPhotoImageProviderTest)

#include "tst_PhotoEditorPhotoImageProvider.moc"
