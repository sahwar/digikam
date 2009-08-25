/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the ManualRenameWidget widget
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "manualrenametest.h"
#include "manualrenametest.moc"

// KDE includes

#include <qtest_kde.h>

// Local includes

#include "parser.h"
#include "manualrenameparser.h"
#include "manualrenamewidget.h"

using namespace Digikam::ManualRename;
using namespace Digikam;

QTEST_KDEMAIN(ManualRenameWidgetTest, GUI)

void ManualRenameWidgetTest::testNumberToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString filename("myfile001.jpg");
    QString cameraName("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("#") << QString("#") << filename << cameraName << curdate << 1
                       << QString("1");

    QTest::newRow("# (index:20)") << QString("#") << filename << cameraName << curdate << 20
                                  << QString("20");

    QTest::newRow("##")  << QString("##") << filename << cameraName << curdate << 2
                         << QString("02");

    QTest::newRow("###") << QString("###") << filename << cameraName << curdate << 4
                         << QString("004");

    QTest::newRow("### (index:40)") << QString("###") << filename << cameraName << curdate << 40
                                    << QString("040");

    QTest::newRow("###_bla_##") << QString("###_bla_##") << filename << cameraName << curdate << 10
                                << QString("010_bla_10");

    QTest::newRow("####{2,3}")  << QString("####{2,3}") << filename << cameraName << curdate << 1
                                << QString("0002");

    QTest::newRow("####{2,3}(10)") << QString("####{2,3}") << filename << cameraName << curdate << 10
                                   << QString("0029");

    QTest::newRow("####{ 2, 3}") << QString("####{ 2, 3}") << filename << cameraName << curdate << 10
                                 << QString("0029");

    QTest::newRow("####{2,3}_bla_## ###") << QString("####{2,3}_bla_## ###") << filename << cameraName << curdate << 1
                                          << QString("0002_bla_01 001");

    QTest::newRow("####{2, 3}_bla_## ###") << QString("####{2, 3}_bla_## ###") << filename << cameraName << curdate << 1
                                           << QString("0002_bla_01 001");

    QTest::newRow("####{ 2, 3}_bla_## ###") << QString("####{ 2, 3}_bla_## ###") << filename << cameraName << curdate << 1
                                            << QString("0002_bla_01 001");

    QTest::newRow("###{100}_bla") << QString("###{100}_bla") << filename << cameraName << curdate << 1
                                  << QString("100_bla");

    QTest::newRow("###{100,}_bla") << QString("###{100,}_bla") << filename << cameraName << curdate << 1
                                   << QString("100_bla");

    QTest::newRow("###{100,   }_bla") << QString("###{100,   }_bla") << filename << cameraName << curdate << 1
                                      << QString("100_bla");

    QTest::newRow("###{100   ,   }_bla") << QString("###{100   ,   }_bla") << filename << cameraName << curdate << 1
                                         << QString("100_bla");
}

void ManualRenameWidgetTest::testNumberToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   cameraName);
    QFETCH(QDateTime, cameraDate);
    QFETCH(int,       index);
    QFETCH(QString,   result);

    ManualRenameParser parser;

    ParseInformation info;
    info.fileName   = filename;
    info.cameraName = cameraName;
    info.datetime   = cameraDate;
    info.index      = index;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void ManualRenameWidgetTest::testFirstLetterOfEachWordUppercaseToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString cameraName("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("myfilename001.jpg") << QString("*") << QString("myfilename001.jpg")
                                       << cameraName << curdate << 1
                                       << QString("Myfilename001");

    QTest::newRow("myfilename001.jpg(token: **)") << QString("**") << QString("myfilename001.jpg")
                                                  << cameraName << curdate << 1
                                                  << QString("Myfilename001Myfilename001");

    QTest::newRow("myfilename001.jpg(token in filename)") << QString("*") << QString("myfilename*001.jpg")
                                                          << cameraName << curdate << 1
                                                          << QString("Myfilename*001");

    QTest::newRow("my image.jpg") << QString("*") << QString("my image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("My Image");

    QTest::newRow("my_image.jpg") << QString("*") << QString("my_image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("My_Image");
}

void ManualRenameWidgetTest::testFirstLetterOfEachWordUppercaseToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   cameraName);
    QFETCH(QDateTime, cameraDate);
    QFETCH(int,       index);
    QFETCH(QString,   result);

    ManualRenameParser parser;

    ParseInformation info;
    info.fileName   = filename;
    info.cameraName = cameraName;
    info.datetime   = cameraDate;
    info.index      = index;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void ManualRenameWidgetTest::testUppercaseToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString cameraName("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("myfilename001.jpg")  << QString("&") << QString("myfilename001.jpg")
                                        << cameraName << curdate << 1
                                        << QString("MYFILENAME001");

    QTest::newRow("my/filename001.jpg") << QString("&") << QString("my/filename001.jpg")
                                        << cameraName << curdate << 1
                                        << QString("FILENAME001");

    QTest::newRow("myfilename001.jpg(token in filename)") << QString("&") << QString("myfilename0&01.jpg")
                                                          << cameraName << curdate << 1
                                                          << QString("MYFILENAME0&01");

    QTest::newRow("my image.jpg") << QString("&") << QString("my image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("MY IMAGE");

    QTest::newRow("my_image.jpg") << QString("&") << QString("my_image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("MY_IMAGE");
}

void ManualRenameWidgetTest::testUppercaseToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   cameraName);
    QFETCH(QDateTime, cameraDate);
    QFETCH(int,       index);
    QFETCH(QString,   result);

    ManualRenameParser parser;

    ParseInformation info;
    info.fileName   = filename;
    info.cameraName = cameraName;
    info.datetime   = cameraDate;
    info.index      = index;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void ManualRenameWidgetTest::testLowercaseToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString cameraName("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("myfilename001.jpg") << QString("%") << QString("MyFileName001.jpg")
                                       << cameraName << curdate << 1
                                       << QString("myfilename001");

    QTest::newRow("myfilename001.jpg(token in filename)") << QString("%") << QString("mYfilenAme0%01.jpg")
                                                  << cameraName << curdate << 1
                                                  << QString("myfilename0%01");

    QTest::newRow("my image.jpg") << QString("%") << QString("MY image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("my image");

    QTest::newRow("my_image.jpg") << QString("%") << QString("mY_Image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("my_image");
}

void ManualRenameWidgetTest::testLowercaseToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   cameraName);
    QFETCH(QDateTime, cameraDate);
    QFETCH(int,       index);
    QFETCH(QString,   result);

    ManualRenameParser parser;

    ParseInformation info;
    info.fileName   = filename;
    info.cameraName = cameraName;
    info.datetime   = cameraDate;
    info.index      = index;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void ManualRenameWidgetTest::testCameraToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString filename("myfile001.jpg");
    QString camname("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("empty")    << QString("[cam]") << filename << QString() << curdate << 1
                              << QString();

    QTest::newRow("   ")      << QString("[cam]") << filename << QString("   ") << curdate << 1
                              << QString();

    QTest::newRow("[cam]")    << QString("[cam]") << filename << camname << curdate << 1
                              << camname;

    QTest::newRow("[Cam]")    << QString("[Cam]") << filename << camname << curdate << 1
                              << camname;

    QTest::newRow("[CAM]")    << QString("[CAM]") << filename << camname << curdate << 1
                              << camname;

    QTest::newRow("[ cam ]")  << QString("[ cam ]") << filename << camname << curdate << 1
                              << QString("[ cam ]");

    QTest::newRow("[camcam]") << QString("[camcam]") << filename << camname << curdate << 1
                              << QString("[camcam]");
}

void ManualRenameWidgetTest::testCameraToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   cameraName);
    QFETCH(QDateTime, cameraDate);
    QFETCH(int,       index);
    QFETCH(QString,   result);

    ManualRenameParser parser;

    ParseInformation info;
    info.fileName   = filename;
    info.cameraName = cameraName;
    info.datetime   = cameraDate;
    info.index      = index;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void ManualRenameWidgetTest::testEmptyParseString()
{
    QString filename("myfilename001.jpg");
    QDateTime curdate = QDateTime::currentDateTime();

    ManualRenameParser parser;

    ParseInformation info;
    info.fileName   = filename;
    info.cameraName = QString();
    info.index      = 1;
    info.datetime   = curdate;

    // test for empty parser string
    QString parsed = parser.parse(QString(), info);
    QCOMPARE(parsed, QString("myfilename001"));

    parsed = parser.parse(QString(""), info);
    QCOMPARE(parsed, QString("myfilename001"));

    parsed = parser.parse(QString("   "), info);
    QCOMPARE(parsed, QString("myfilename001"));

    // the following is not invalid
    parsed = parser.parse(QString("  %_##"), info);
    QCOMPARE(parsed, QString("  myfilename001_01"));
}

void ManualRenameWidgetTest::testSetters()
{
    ManualRenameWidget mrename;

    mrename.setText("this is a test");
    QVERIFY(!mrename.text().isEmpty());
    QCOMPARE(mrename.text(), QString("this is a test"));
}
