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

#include <time.h>
#include <utime.h>

const int SCALED_LOAD_FLOOR = 360;

class PhotoEditorPhotoImageProviderTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();
    void initTestCase();

    void testEmptyOrInvalid();
    void testRotation();
    void testCache();
    void testCacheSizes();

private:        
    PhotoImageProvider *m_provider;
    QTemporaryDir m_workingDir;
};

void PhotoEditorPhotoImageProviderTest::initTestCase()
{
    QDir rc = QDir(":/assets/");
    QDir dest = QDir(m_workingDir.path());
    Q_FOREACH(const QString &name, rc.entryList())
    {
        QFile::copy(rc.absoluteFilePath(name), dest.absoluteFilePath(name));
        QFile::setPermissions(dest.absoluteFilePath(name),
                              QFile::WriteOwner | QFile::ReadOwner);
    }
}

void PhotoEditorPhotoImageProviderTest::init()
{
    m_provider = new PhotoImageProvider();
    m_provider->setEmitCacheSignals(true);
}

void PhotoEditorPhotoImageProviderTest::cleanup()
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
    QSize imageSize(400, 267);

    // Work on a copy to avoid disturbing other tests
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("testcache.jpg");
    QFile::remove(path);
    QFile::copy(source.absoluteFilePath("windmill.jpg"), path);

    // Set the file modification time to a date back in the past
    // to prevent problems with low file system modification time resolution.
    struct utimbuf tm;
    tm.actime = 1;
    tm.modtime = 1;
    utime(path.toUtf8().constData(), &tm);

    QSignalSpy spyHit(m_provider, SIGNAL(cacheHit(QString,QSize)));
    QSignalSpy spyMiss(m_provider, SIGNAL(cacheMiss(QString,QSize,bool)));
    QSignalSpy spyAdd(m_provider, SIGNAL(cacheAdd(QString,QSize,QSize)));

    // First request an image so that it gets cached
    QImage image = m_provider->requestImage(path, 0, QSize());
    QVERIFY(!image.isNull());
    QVERIFY(image.size() == imageSize);
    QVERIFY(spyHit.count() == 0);
    QVERIFY(spyMiss.count() == 1);
    QVERIFY(spyMiss.front().at(0) == path);
    QVERIFY(spyAdd.count() == 1);
    QVERIFY(spyAdd.front().at(0) == path);

    // Verify that it is there
    image = m_provider->requestImage(path, 0, QSize());
    QVERIFY(spyHit.count() == 1);
    QVERIFY(spyHit.front().at(0) == path);

    // Copy another file with different rotation into the same file
    QFile main(path);
    main.open(QIODevice::WriteOnly);

    QFile other(source.absoluteFilePath("windmill_rotated_90.jpg"));
    other.open(QIODevice::ReadOnly);

    main.write(other.readAll());
    main.close();
    other.close();

    // Verify that we are getting a miss and that it is a stale cache miss
    image = m_provider->requestImage(path, 0, QSize());
    QVERIFY(spyMiss.count() == 2);
    QVERIFY(spyMiss.at(1).at(0) == path);
    QVERIFY(spyMiss.at(1).at(2) == true); // is stale
    QVERIFY(spyAdd.count() == 2);
    QVERIFY(spyAdd.at(1).at(0) == path);
    QVERIFY(spyAdd.at(1).at(2) == imageSize); // cached image size

    // Verify that the image we are getting has the correct rotation, and
    // therefore matches the new data in the file
    QVERIFY(image.size() == imageSize.transposed());
}

void PhotoEditorPhotoImageProviderTest::testCacheSizes()
{
    QSize imageSize(1408, 768);

    // Work on a copy to avoid disturbing other tests
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("testcache.jpg");
    QFile::remove(path);
    QFile::copy(source.absoluteFilePath("thorns.jpg"), path);

    // Set the file modification time to a date back in the past
    // to prevent problems with low file system modification time resolution.
    struct utimbuf tm;
    tm.actime = 1;
    tm.modtime = 1;
    utime(path.toUtf8().constData(), &tm);

    QSignalSpy spyAdd(m_provider, SIGNAL(cacheAdd(QString,QSize,QSize)));
    QSignalSpy spyHit(m_provider, SIGNAL(cacheHit(QString,QSize)));

    QSize small(1408 / 4, 768 / 4);

    // Request a size smaller than the image, and verify the smaller version
    // gets cached
    QImage image = m_provider->requestImage(path, 0, small);
    QVERIFY(spyAdd.count() == 1);
    QVERIFY(spyAdd.at(0).at(2) == small);
    QVERIFY(image.size() == small);

    // Request the full size and verify that the image is re-cached and the
    // full size is returned
    image = m_provider->requestImage(path, 0, imageSize);
    QVERIFY(spyAdd.count() == 2);
    QVERIFY(spyAdd.at(1).at(2) == imageSize);
    QVERIFY(image.size() == imageSize);

    // Verify that requesting a smaller size after caching the full size will
    // result in a cache hit
    image = m_provider->requestImage(path, 0, small);
    QVERIFY(spyHit.count() == 1);
    QVERIFY(image.size() == small);
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
