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

#include "photo-data.h"

#include <QTest>
#include <QSignalSpy>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

class PhotoEditorPhotoTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testBasicProperties();
    void testOrientation();
    void testRefresh();
    void testRotate();

    void cleanupTestCase();

private:        
    QTemporaryDir m_workingDir;
};

void PhotoEditorPhotoTest::initTestCase()
{
    QDir rc = QDir(":/assets/");
    QDir dest = QDir(m_workingDir.path());
    Q_FOREACH(const QString &name, rc.entryList())
    {
        QFile::copy(rc.absoluteFilePath(name), dest.absoluteFilePath(name));
        QFile::setPermissions(dest.absoluteFilePath(name),
                              QFile::WriteOwner | QFile::ReadOwner);
    }
    m_workingDir.setAutoRemove(false);
}

void PhotoEditorPhotoTest::cleanupTestCase()
{
    // temporary dir will be deleted when m_workingDir goes out of scope
}

void PhotoEditorPhotoTest::testBasicProperties()
{
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("windmill.jpg");

    PhotoData photo;
    photo.setPath(path);
    QVERIFY(photo.path() == path);
    QVERIFY(photo.file() == QFileInfo(path));
    QVERIFY(photo.fileFormat() == "jpeg");
    QVERIFY(photo.fileFormatHasMetadata() == true);
    QVERIFY(photo.fileFormatHasOrientation() == true);
}

void PhotoEditorPhotoTest::testOrientation()
{
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("windmill.jpg");

    PhotoData photo;
    photo.setPath(path);
    QVERIFY(photo.orientation() == TOP_LEFT_ORIGIN);

    PhotoData photo2;
    QString path2 = source.absoluteFilePath("windmill_rotated_90.jpg");
    photo2.setPath(path2);
    QVERIFY(photo2.orientation() == RIGHT_TOP_ORIGIN);

    // Test updating the original PhotoData by changing path
    photo.setPath(path2);
    QVERIFY(photo.orientation() == RIGHT_TOP_ORIGIN);
}

void PhotoEditorPhotoTest::testRefresh()
{
    // Work on a copy to avoid disturbing other tests
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("testrefresh.jpg");
    qDebug() << QFile::copy(source.absoluteFilePath("windmill.jpg"), path);

    // Load the file.
    PhotoData photo;
    photo.setPath(path);
    Orientation orientation = photo.orientation();

    // Now overwrite it with another file with different rotation and verify
    // the new data is correct
    QFile::remove(path);
    QFile::copy(source.absoluteFilePath("windmill_rotated_90.jpg"), path);

    photo.refreshFromDisk();
    QVERIFY(orientation != photo.orientation());
}

void PhotoEditorPhotoTest::testRotate()
{
    // Work on a copy to avoid disturbing other tests
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("testrotate.jpg");
    QFile::copy(source.absoluteFilePath("windmill.jpg"), path);

    PhotoData photo;
    photo.setPath(path);
    QVERIFY(photo.path() == path);
    QVERIFY(photo.orientation() == TOP_LEFT_ORIGIN);

    QSignalSpy spy(&photo, SIGNAL(busyChanged()));
    photo.rotateRight();
    spy.wait(5000);

    QVERIFY(photo.orientation() == RIGHT_TOP_ORIGIN);

    spy.clear();
    photo.rotateRight();
    spy.wait(5000);

    QVERIFY(photo.orientation() == BOTTOM_RIGHT_ORIGIN);

    spy.clear();
    photo.rotateRight();
    spy.wait(5000);

    QVERIFY(photo.orientation() == LEFT_BOTTOM_ORIGIN);

    spy.clear();
    photo.rotateRight();
    spy.wait(5000);

    QVERIFY(photo.orientation() == TOP_LEFT_ORIGIN);
}



QTEST_MAIN(PhotoEditorPhotoTest)

#include "tst_PhotoEditorPhoto.moc"
