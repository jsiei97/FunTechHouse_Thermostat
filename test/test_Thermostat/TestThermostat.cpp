/**
 * @file TestThermostat.cpp
 * @author Johan Simonsson
 * @brief Testfile for Thermostat
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

#include "Thermostat.h"

class TestThermostat : public QObject
{
    Q_OBJECT

    private:
    public:

    private slots:
        void test_getStageCount();
        void test_getStageCount_data();

        void test_getStageOut();
        void test_setStageOut();

        void test_valueTimeToSend();
};


#define PRINT_DATA() do{ \
qDebug() << "Stages  :" << thermostat.stages; \
qDebug() << "Value   :" << thermostat.value    << "sent:" << thermostat.valueSent; \
qDebug() << "Setpoint:" << thermostat.setpoint << "sent:" << thermostat.setpointSent; \
qDebug() << "output  :" << thermostat.stageOut << "sent:" << thermostat.stageOutSent ; \
}while(0);


void TestThermostat::test_getStageCount_data()
{
    QTest::addColumn<int>("value");

    for(int i=0; i<6; i++)
    {
        QTest::newRow("value") <<  i;
    }

};

void TestThermostat::test_getStageCount()
{
    QFETCH(int, value);

    Thermostat thermostat(value);
    int stageCount = thermostat.getStageCount();
    QCOMPARE(stageCount, value);
};

void TestThermostat::test_getStageOut()
{
    Thermostat thermostat(3); // Stage: 0,1,2

    thermostat.stageOut = 0x1;
    QCOMPARE(thermostat.getStageOut(0), true);
    QCOMPARE(thermostat.getStageOut(1), false);

    thermostat.stageOut = 0x2;
    QCOMPARE(thermostat.getStageOut(0), false);
    QCOMPARE(thermostat.getStageOut(1), true);

    thermostat.stageOut = 0x3;
    QCOMPARE(thermostat.getStageOut(0), true);
    QCOMPARE(thermostat.getStageOut(1), true);

    thermostat.stageOut = 0x4;
    QCOMPARE(thermostat.getStageOut(0), false);
    QCOMPARE(thermostat.getStageOut(1), false);
    QCOMPARE(thermostat.getStageOut(2), true);

    thermostat.stageOut = 0x7;
    QCOMPARE(thermostat.getStageOut(0), true);
    QCOMPARE(thermostat.getStageOut(1), true);
    QCOMPARE(thermostat.getStageOut(2), true);

    //stageCount is 3, so deny the fourth
    thermostat.stageOut = 0xF;
    QCOMPARE(thermostat.getStageOut(3), false);
    QCOMPARE(thermostat.getStageOut(4), false);

    QCOMPARE(thermostat.getStageOut(0), true);
    QCOMPARE(thermostat.getStageOut(1), true);
    QCOMPARE(thermostat.getStageOut(2), true);

}

void TestThermostat::test_setStageOut()
{
    Thermostat thermostat(4); // Stage: 0,1,2,3

    QCOMPARE(thermostat.stageOut , (uint8_t)0x0);

    QVERIFY(thermostat.setStageOut(0, true));
    QCOMPARE(thermostat.stageOut , (uint8_t)0x1);
    QVERIFY(thermostat.setStageOut(0, false));
    QCOMPARE(thermostat.stageOut , (uint8_t)0x0);

    QVERIFY(thermostat.setStageOut(0, true));
    QVERIFY(thermostat.setStageOut(1, true));
    QCOMPARE(thermostat.stageOut , (uint8_t)0x3);

    QVERIFY(thermostat.setStageOut(0, false));
    QCOMPARE(thermostat.stageOut , (uint8_t)0x2);

    QVERIFY(thermostat.setStageOut(0, false));
    QCOMPARE(thermostat.stageOut , (uint8_t)0x2);

    //Do not allow to active stage output for non active stages
    thermostat.stageOut = 0x0;
    QCOMPARE(thermostat.setStageOut(4, true), false);
    QCOMPARE(thermostat.stageOut , (uint8_t)0x0);
}

void TestThermostat::test_valueTimeToSend()
{
    Thermostat thermostat(3);

    thermostat.setSetpoint(50.0);
    QCOMPARE(thermostat.getSetpoint() , 50.0);
    thermostat.valueIsSent();

    //Value lower than setpoint, should turn on the output.
    thermostat.valueTimeToSend(40.0);
    QCOMPARE(thermostat.getStageOut(0), true);
    QCOMPARE(thermostat.getStageOut(1), false);
    QCOMPARE(thermostat.getStageOut(2), false);

    thermostat.valueIsSent();

    //Value higher than setpoint and we turn off.
    QCOMPARE(thermostat.valueTimeToSend(60.0), true);
    QCOMPARE(thermostat.getStageOut(0), false);

    //thermostat.valueIsSent();
    //PRINT_DATA();
    QCOMPARE(thermostat.valueTimeToSend(60.0), true);
    //PRINT_DATA();
    thermostat.valueIsSent();
    //PRINT_DATA();
    QCOMPARE(thermostat.valueTimeToSend(60.0), false);

}

QTEST_MAIN(TestThermostat)
#include "TestThermostat.moc"
