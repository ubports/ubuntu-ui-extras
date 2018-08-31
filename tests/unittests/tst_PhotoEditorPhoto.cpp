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

#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include "photo-metadata.h"

using namespace PhotoEditor;

class PhotoEditorPhotoTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testBasicProperties();
    void testOrientation();
    void testSaveImage();
    void testSetOrientationAndSave();

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

    PhotoMetadata photo;
    photo.setFileName(path);
    QVERIFY(photo.fileName() == path);
    QVERIFY(photo.orientation() == TOP_LEFT_ORIGIN);
    QVERIFY(photo.saving() == false);
}

void PhotoEditorPhotoTest::testOrientation()
{
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("windmill.jpg");

    PhotoMetadata photo;
    photo.setFileName(path);
    QVERIFY(photo.orientation() == TOP_LEFT_ORIGIN);

    PhotoMetadata photo2;
    QString path2 = source.absoluteFilePath("windmill_rotated_90.jpg");
    photo2.setFileName(path2);
    QVERIFY(photo2.orientation() == RIGHT_TOP_ORIGIN);

    // Test updating the original PhotoMetadata by changing path
    photo.setFileName(path2);
    QVERIFY(photo.orientation() == RIGHT_TOP_ORIGIN);
}

void PhotoEditorPhotoTest::testSaveImage()
{
    // Work on a copy to avoid disturbing other tests
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("savetestcopy.png");
    QFile::remove(path);
    QFile::copy(source.absoluteFilePath("savetest.png"), path);

    // Load the file.
    PhotoMetadata photo;
    photo.setFileName(path);

    // Now overwrite it with another QImage and verify the new data is correct
    QImage otherImage(source.absoluteFilePath("windmill.jpg"));

    QVERIFY(photo.saving() == false);
    photo.saveImage(otherImage);
    QVERIFY(photo.saving() == true);
    QTRY_VERIFY(photo.saving() == false);

    QImage result(path);
    QCOMPARE(result, otherImage);
}

void PhotoEditorPhotoTest::testSetOrientationAndSave()
{
    // Work on a copy to avoid disturbing other tests
    QDir source = QDir(m_workingDir.path());
    QString path = source.absoluteFilePath("windmillcopy.jpg");
    QFile::remove(path);
    QFile::copy(source.absoluteFilePath("windmill.jpg"), path);

    PhotoMetadata photo;
    photo.setFileName(path);
    QVERIFY(photo.orientation() == TOP_LEFT_ORIGIN);

    photo.setOrientation(RIGHT_TOP_ORIGIN);
    QVERIFY(photo.orientation() == RIGHT_TOP_ORIGIN);

    QVERIFY(photo.saving() == false);
    photo.save();
    QVERIFY(photo.saving() == true);
    QTRY_VERIFY(photo.saving() == false);

    // Reread the metadata and check the orientation
    PhotoMetadata otherPhoto;
    otherPhoto.setFileName(path);
    QVERIFY(otherPhoto.orientation() == RIGHT_TOP_ORIGIN);
}


QTEST_MAIN(PhotoEditorPhotoTest)

#include "tst_PhotoEditorPhoto.moc"
