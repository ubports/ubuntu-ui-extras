/*
 * Copyright (C) 2012 Canonical, Ltd.
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

#include "imageresizer.h"

#include <QTest>
#include <QSignalSpy>
#include <QDebug>

const QString testFile = ":/assets/testfile.jpg";
const QString testFilePortrait = ":/assets/testfile_portrait.jpg";

class ShareImageResizerTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testProperties();
    void testResizeImage();
    void testInvalidFilename();
    void testInvalidSide();
    void testInvalidQuality();

    void cleanupTestCase();

private:
    ImageResizer *m_resizer;
};

void ShareImageResizerTest::initTestCase()
{
    m_resizer = new ImageResizer(this);
}

void ShareImageResizerTest::cleanupTestCase()
{
    delete m_resizer;
}

void ShareImageResizerTest::testProperties()
{
    QSignalSpy fileSpy(m_resizer, SIGNAL(fileChanged()));
    QSignalSpy largestSideSpy(m_resizer, SIGNAL(largestSideChanged()));
    QSignalSpy qualitySpy(m_resizer, SIGNAL(qualityChanged()));
    QSignalSpy resizedFileSpy(m_resizer, SIGNAL(resizedFileChanged()));

    m_resizer->setFile(testFile);
    QVERIFY(fileSpy.count() == 1);
    QVERIFY(resizedFileSpy.count() == 1);

    m_resizer->setLargestSide(1024);
    QVERIFY(largestSideSpy.count() == 1);
    QVERIFY(resizedFileSpy.count() == 2);

    m_resizer->setQuality(110);
    QVERIFY(qualitySpy.count() == 1);
    QVERIFY(resizedFileSpy.count() == 3);


    QVERIFY(m_resizer->file() == testFile);
    QVERIFY(m_resizer->largestSide() == 1024);
    QVERIFY(m_resizer->quality() == 100);


    // Set to the same stuff again
    m_resizer->setFile(testFile);
    m_resizer->setLargestSide(1024);
    m_resizer->setQuality(110);

   // Test if all the changed signals are NOT emitted for a second time
    QVERIFY(fileSpy.count() == 1);
    QVERIFY(largestSideSpy.count() == 1);
    QVERIFY(qualitySpy.count() == 1);
}

void ShareImageResizerTest::testResizeImage()
{
    QSignalSpy resizedFileSpy(m_resizer, SIGNAL(resizedFileChanged()));

    m_resizer->setLargestSide(128);
    QVERIFY(resizedFileSpy.count() == 1);

    QImage image(m_resizer->resizedFile());
    QVERIFY(image.width() == 128);
    QVERIFY(image.height() < 128);

    m_resizer->setLargestSide(55000);
    QImage original(m_resizer->file());
    image = QImage(m_resizer->resizedFile());
    QVERIFY(image.width() == original.width());
    QVERIFY(image.height() == original.height());

    m_resizer->setLargestSide(128);
    m_resizer->setFile(testFilePortrait);
    image = QImage(m_resizer->resizedFile());
    QVERIFY(m_resizer->file() == testFilePortrait);
    QVERIFY(image.height() == 128);
    QVERIFY(image.width() < 128);

    m_resizer->setLargestSide(55000);
    original = QImage(m_resizer->file());
    image = QImage(m_resizer->resizedFile());
    QVERIFY(image.width() == original.width());
    QVERIFY(image.height() == original.height());
}

void ShareImageResizerTest::testInvalidFilename()
{
    QSignalSpy resizedFileSpy(m_resizer, SIGNAL(resizedFileChanged()));

    m_resizer->setFile(testFile + QLatin1String("invalid"));
    QVERIFY(m_resizer->resizedFile().isEmpty());
}

void ShareImageResizerTest::testInvalidSide()
{
    QSignalSpy largestSideSpy(m_resizer, SIGNAL(largestSideChanged()));

    m_resizer->setLargestSide(-100);
    QVERIFY(m_resizer->largestSide() == 1);
}

void ShareImageResizerTest::testInvalidQuality()
{
    QSignalSpy qualitySpy(m_resizer, SIGNAL(qualityChanged()));

    m_resizer->setQuality(-100);
    QVERIFY(m_resizer->quality() == 0);

    m_resizer->setQuality(2000);
    QVERIFY(m_resizer->quality() == 100);
}

QTEST_MAIN(ShareImageResizerTest)

#include "tst_ShareImageResizer.moc"
