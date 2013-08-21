/**
 * @file TestStringHelp.cpp
 * @author Johan Simonsson
 * @brief Testfile for StringHelp
 */

/*
 * Copyright (C) 2013 Johan Simonsson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore>
#include <QtTest>
#include <QString>

#include "StringHelp.h"

class TestStringHelp : public QObject
{
    Q_OBJECT

    private:
    public:

    private slots:
        void test_add_data();
        void test_add();
};

void TestStringHelp::test_add_data()
{
    QTest::addColumn<double>("value");
    QTest::addColumn<int>("outInt");
    QTest::addColumn<int>("outDec");

    QTest::newRow("value  1.030") <<  1.03 <<  1 <<  3;
    QTest::newRow("value  1.300") <<  1.30 <<  1 << 30;

    QTest::newRow("value 10  ") << 10.0 << 10 <<  0;
    QTest::newRow("value  5.4") <<  5.4 <<  5 << 40;
    QTest::newRow("value  0.3") <<  0.3 <<  0 << 30;

    QTest::newRow("value -10  ") << -10.0 << -10 <<  0;
    QTest::newRow("value  -5.4") <<  -5.4 <<  -5 << 40;
    QTest::newRow("value  -0.3") <<  -0.3 <<  -0 << 30;

    QTest::newRow("value   5.6")  <<   5.6  <<   5 << 60;
    QTest::newRow("value  42.62") <<  42.62 <<  42 << 62;

    QTest::newRow("value  -5.6")  <<  -5.6  <<  -5 << 60;
    QTest::newRow("value -42.62") << -42.62 << -42 << 62;

    for( int i = 5480; i <= 5520 ; i += 1 )
    {
        double value = i;
        value /= 100;
        QString str = QString("%1").arg(value, 0, 'f', 2);
        QStringList list = str.split(".");
        //qDebug() << str << list.at(0) << list.at(1) << value;

        bool ok;
        QTest::newRow("value for") << value << list.at(0).toInt(&ok, 10) << list.at(1).toInt(&ok, 10);
    }
    
    //for( int i = 54990; i <= 55010 ; i += 1 )
    //55005..55010 rounds ok 55.006 -> 55 01
    //but this test thinks   55.006 -> 55 00 (since I /=10)

    for( int i = 54990; i <= 55004 ; i += 1 )
    {
        double value = i;
        value /= 1000;
        QString str = QString("%1").arg(value, 0, 'f', 3);
        QStringList list = str.split(".");
        //qDebug() << str << list.at(0) << list.at(1) << value;

        bool ok;
        int intPart = list.at(0).toInt(&ok, 10);
        int decPart = list.at(1).toInt(&ok, 10);
        decPart /= 10;

        //qDebug() << str << intPart << decPart << value;
        QTest::newRow("value for") << value << intPart << decPart;
    }
};

void TestStringHelp::test_add()
{
    QFETCH(double, value);
    QFETCH(int, outInt);
    QFETCH(int, outDec);

    int intPart = 0;
    int decPart = 0;
    StringHelp::splitDouble(value, &intPart, &decPart);

    if( (intPart != outInt) || (decPart != outDec) )
    {
        qDebug() << "CORRECT:" << value << outInt  << outDec;
        qDebug() << "WRONG  :" << value << intPart << decPart;
        QFAIL("values not the same");
    }

    QCOMPARE(intPart, outInt);
    QCOMPARE(decPart, outDec);
}


QTEST_MAIN(TestStringHelp)
#include "TestStringHelp.moc"
