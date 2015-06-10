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
    void testNoResize();
    void testWithResize();

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
}

void PhotoEditorPhotoImageProviderTest::cleanup()
{
    // temporary dir will be deleted when m_workingDir goes out of scope
    delete m_provider;
}

void PhotoEditorPhotoImageProviderTest::testNoResize()
{
    QSize imageSize(400, 267);

    // Work on a copy to avoid disturbing other tests
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("testcache.jpg");
    QFile::remove(path);
    QFile::copy(source.absoluteFilePath("windmill.jpg"), path);

    QImage image = m_provider->requestImage(path, 0, QSize());
    QVERIFY(!image.isNull());
    QVERIFY(image.size() == imageSize);
}

void PhotoEditorPhotoImageProviderTest::testWithResize()
{
    QSize imageSize(1408, 768);

    // Work on a copy to avoid disturbing other tests
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("testcache.jpg");
    QFile::remove(path);
    QFile::copy(source.absoluteFilePath("thorns.jpg"), path);

    QSize small(1408 / 4, 768 / 4);

    // Request a size smaller than the image
    QImage image = m_provider->requestImage(path, 0, small);
    QVERIFY(image.size() == small);

    // Request the full size
    image = m_provider->requestImage(path, 0, imageSize);
    QVERIFY(image.size() == imageSize);

    // Verify that requesting a smaller size still works
    image = m_provider->requestImage(path, 0, small);
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
