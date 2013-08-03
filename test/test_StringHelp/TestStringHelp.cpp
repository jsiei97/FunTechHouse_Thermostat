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
};

void TestStringHelp::test_add()
{
    QFETCH(double, value);
    QFETCH(int, outInt);
    QFETCH(int, outDec);

    int intPart = 0;
    int decPart = 0;
    StringHelp::splitDouble(value, &intPart, &decPart);

    QCOMPARE(intPart, outInt);
    QCOMPARE(decPart, outDec);
}


QTEST_MAIN(TestStringHelp)
#include "TestStringHelp.moc"
