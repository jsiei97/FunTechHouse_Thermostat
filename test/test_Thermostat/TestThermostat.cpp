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
        void test_incStageOut();

        void test_valueTimeToSend();
        void test_checkSetpoint();

        void test_getValueString();
        void test_getValueString_data();

        void test_getOutValue();
        void test_getOutValue_data();

        void test_alarmLowTimeToSend();
        void test_alarmHighTimeToSend();
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

void TestThermostat::test_incStageOut()
{
    Thermostat thermostat(4); // Stage: 0,1,2,3

    QCOMPARE(thermostat.stageOut , (uint8_t)0x0);
    thermostat.incStageOut();
    QCOMPARE(thermostat.stageOut , (uint8_t)0x1);
    thermostat.incStageOut();
    QCOMPARE(thermostat.stageOut , (uint8_t)0x3);
    thermostat.incStageOut();
    QCOMPARE(thermostat.stageOut , (uint8_t)0x7);

    //No more inc since we maxed out the stages...
    //thermostat.incStageOut();
    //QCOMPARE(thermostat.stageOut , (uint8_t)0x7);


    Thermostat th2(1);
    QCOMPARE(th2.stageOut , (uint8_t)0x0);
    th2.incStageOut();
    QCOMPARE(th2.stageOut , (uint8_t)0x1);
    //th2.incStageOut();
    //QCOMPARE(th2.stageOut , (uint8_t)0x1);

}

void TestThermostat::test_valueTimeToSend()
{
    Thermostat thermostat(3);

    thermostat.setSetpoint(50.0, 0.0);
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
    QCOMPARE(thermostat.valueTimeToSend(40.0), true);
    QCOMPARE(thermostat.getStageOut(0), true);
    QCOMPARE(thermostat.getStageOut(1), false);
    QCOMPARE(thermostat.getStageOut(2), false);

    thermostat.valueIsSent();

    //Above was 1 low value count
    //And then stop 1 before..
    for(int step=0; step<(LOW_VALUE_COUNT_MAX-1); step++)
    {
        //qDebug() << "step" << step;
        QCOMPARE(thermostat.valueTimeToSend(40.0), false);
        QCOMPARE(thermostat.getStageOut(0), true);
        QCOMPARE(thermostat.getStageOut(1), false);
        QCOMPARE(thermostat.getStageOut(2), false);
    }

    QCOMPARE(thermostat.valueTimeToSend(40.0), true);
    QCOMPARE(thermostat.getStageOut(0), true);
    QCOMPARE(thermostat.getStageOut(1), true);
    QCOMPARE(thermostat.getStageOut(2), false);

    thermostat.valueIsSent();

    //And then stop 1 before..
    for(int step=0; step<(LOW_VALUE_COUNT_MAX-1); step++)
    {
        //qDebug() << "step" << step;
        QCOMPARE(thermostat.valueTimeToSend(40.0), false);
        QCOMPARE(thermostat.getStageOut(0), true);
        QCOMPARE(thermostat.getStageOut(1), true);
        QCOMPARE(thermostat.getStageOut(2), false);
    }

    QCOMPARE(thermostat.valueTimeToSend(40.0), true);
    QCOMPARE(thermostat.getStageOut(0), true);
    QCOMPARE(thermostat.getStageOut(1), true);
    QCOMPARE(thermostat.getStageOut(2), true);


    thermostat.valueIsSent();

    //Value higher than setpoint and we turn off.
    QCOMPARE(thermostat.valueTimeToSend(60.0), true);
    QCOMPARE(thermostat.getStageOut(0), false);
    QCOMPARE(thermostat.getStageOut(1), false);
    QCOMPARE(thermostat.getStageOut(2), false);


    //Then we have the same data, 
    //and see that we timeout and send anyway!
    thermostat.valueIsSent();
    for(int i=1; i<=ALWAYS_SEND_CNT; i++)
    {
        if(thermostat.valueTimeToSend(60.0))
        {
            qDebug() << "Error we should not send now" << i;
            QFAIL("Error valueSendCnt wrong");
        }
    }
    QCOMPARE(thermostat.valueTimeToSend(60.0), true);


    //Check that value diff is correct, both for bigger and smaller values.
    thermostat.valueIsSent();
    thermostat.valueDiffMax = 1.0;
    QCOMPARE(thermostat.valueTimeToSend(60.0), false);
    QCOMPARE(thermostat.valueTimeToSend(59.5), false);
    QCOMPARE(thermostat.valueTimeToSend(58.5), true);

    QCOMPARE(thermostat.valueTimeToSend(50.0), true);
    thermostat.valueIsSent();
    QCOMPARE(thermostat.valueTimeToSend(50.5), false);
    QCOMPARE(thermostat.valueTimeToSend(51.5), true);

}


/**
 * Test the setpoint hysteresis is working.
 * Walk the value up and down and look at the output.
 */
void TestThermostat::test_checkSetpoint()
{
    Thermostat thermostat(1);
    double value;

    thermostat.setSetpoint(50.0, 5.0);
    thermostat.valueIsSent();

    //Check that it is on all the up to the setpoint.
    value = 40.0;
    while(value < 50.0)
    {
        thermostat.valueTimeToSend(value);
        if(true != thermostat.getStageOut(0))
        {
            qDebug() << "FAIL keep on 1" << value;
            QFAIL("FAIL");
        }
        value++;
    }

    thermostat.valueTimeToSend(value);
    if(false != thermostat.getStageOut(0))
    {
        qDebug() << "FAIL turn off 1" << value;
        QFAIL("FAIL");
    }

    value = 55.0;
    while(value >= 45.0)
    {
        thermostat.valueTimeToSend(value);
        if(false != thermostat.getStageOut(0))
        {
            qDebug() << "FAIL keep off" << value;
            QFAIL("FAIL");
        }
        value--;
    }

    thermostat.valueTimeToSend(value);
    if(true != thermostat.getStageOut(0))
    {
        qDebug() << "FAIL turn on 2" << value;
        QFAIL("FAIL");
    }

    while(value < 50.0)
    {
        thermostat.valueTimeToSend(value);
        if(true != thermostat.getStageOut(0))
        {
            qDebug() << "FAIL keep on 1" << value;
            QFAIL("FAIL");
        }
        value++;
    }

    thermostat.valueTimeToSend(value);
    if(false != thermostat.getStageOut(0))
    {
        qDebug() << "FAIL turn off 1" << value;
        QFAIL("FAIL");
    }
}


void TestThermostat::test_getValueString_data()
{
    QTest::addColumn<QString>("valueString");
    QTest::addColumn<double>("temperature");
    QTest::addColumn<double>("setpoint");
    QTest::addColumn<unsigned int>("stageCount");
    QTest::addColumn<unsigned int>("out"); ///< 0..100%

    QTest::newRow("Test")
        << "value=20.00 ; setpoint=40.00 ; output=100%"
        << 20.0 << 40.0 << (unsigned int)1 << (unsigned int)100;

    QTest::newRow("Test")
        << "value=60.04 ; setpoint=39.50 ; output=000%"
        << 60.04 << 39.5 << (unsigned int)1 << (unsigned int)0;
}

void TestThermostat::test_getValueString()
{
    QFETCH(QString, valueString);
    QFETCH(double, temperature);
    QFETCH(double, setpoint);
    QFETCH(unsigned int, stageCount);
    QFETCH(unsigned int, out);

    Thermostat thermostat(stageCount);
    thermostat.setSetpoint(setpoint, 0.0);
    thermostat.valueTimeToSend(temperature);

    QString result(thermostat.getValueString());
    //qDebug() << valueString << result;
    QCOMPARE(valueString, result);
}


void TestThermostat::test_getOutValue_data()
{
    QTest::addColumn<uint8_t>("outStages");
    QTest::addColumn<unsigned int>("stageCount");
    QTest::addColumn<unsigned int>("out"); ///< 0..100%

    QTest::newRow("Test") << (uint8_t)0x0 << (unsigned int)4 << (unsigned int)0;
    QTest::newRow("Test") << (uint8_t)0x1 << (unsigned int)4 << (unsigned int)25;
    QTest::newRow("Test") << (uint8_t)0x3 << (unsigned int)4 << (unsigned int)50;
    QTest::newRow("Test") << (uint8_t)0x7 << (unsigned int)4 << (unsigned int)75;
    QTest::newRow("Test") << (uint8_t)0xF << (unsigned int)4 << (unsigned int)100;

    QTest::newRow("Test") << (uint8_t)0x0 << (unsigned int)3 << (unsigned int)0;
    QTest::newRow("Test") << (uint8_t)0x1 << (unsigned int)3 << (unsigned int)33;
    QTest::newRow("Test") << (uint8_t)0x3 << (unsigned int)3 << (unsigned int)66;
    QTest::newRow("Test") << (uint8_t)0x7 << (unsigned int)3 << (unsigned int)100;

    QTest::newRow("Test") << (uint8_t)0x0 << (unsigned int)2 << (unsigned int)0;
    QTest::newRow("Test") << (uint8_t)0x1 << (unsigned int)2 << (unsigned int)50;
    QTest::newRow("Test") << (uint8_t)0x3 << (unsigned int)2 << (unsigned int)100;

    QTest::newRow("Test") << (uint8_t)0x0 << (unsigned int)1 << (unsigned int)0;
    QTest::newRow("Test") << (uint8_t)0x1 << (unsigned int)1 << (unsigned int)100;
}

void TestThermostat::test_getOutValue()
{
    QFETCH(uint8_t, outStages);
    QFETCH(unsigned int, stageCount);
    QFETCH(unsigned int, out);

    Thermostat thermostat(stageCount);
    thermostat.stageOut = outStages;

    QCOMPARE(out, thermostat.getOutValue());
}


void TestThermostat::test_alarmLowTimeToSend()
{
    Thermostat thermostat(1);

    double value = 20.0;
    double setpoint = 40.0;
    thermostat.setSetpoint(setpoint, 0.0);
    thermostat.setAlarmLevels(true, 10.0, true, 10.0);
    thermostat.valueTimeToSend(value);

    QCOMPARE(thermostat.alarmLowTimeToSend(), false);

    for(int i=0; i<(FIRST_ALARM_ALLOWED-1); i++)
    {
        thermostat.valueTimeToSend(value);
        if(thermostat.alarmLowTimeToSend())
        {
            qDebug() << "Error: to early for alarmLow"<< i;
            QFAIL("FAIL");
        }
    }

    //PRINT_DATA();
    QCOMPARE(thermostat.alarmLowTimeToSend(), true);
    thermostat.alarmLowIsSent();

    //Now the alarm is active
    //Let's rise the value until it stops.
    while(value < setpoint+5)
    {
        thermostat.valueTimeToSend(value);
        if(false != thermostat.alarmLowTimeToSend())
        {
            //PRINT_DATA();
            qDebug() << "Already sent, so not more" << value << setpoint;;
            QFAIL("FAIL");
        }
        value++;
    }

    //PRINT_DATA();
    while(!thermostat.alarmLowTimeToSend())
    {
        value--;
        thermostat.valueTimeToSend(value);
    }

    if(value > (setpoint-thermostat.alarmLevelLow))
    {
        qDebug() << "Alarm triggered to early" << value << setpoint;
        QFAIL("FAIL");
    }

    //PRINT_DATA();
    QVERIFY(thermostat.alarmLowTimeToSend());

    
    QString alarmStr1 = QString("Alarm Low ; value=%1 ; alarm=%2 ; setpoint=%3 ; output=100%")
        .arg(value, 0, 'f', 2)
        .arg((setpoint-thermostat.alarmLevelLow), 0, 'f', 2)
        .arg(setpoint, 0, 'f', 2);

    QString alarmStr2(thermostat.getAlarmLowString());
    //qDebug() << alarmStr1;
    //qDebug() << alarmStr2;
    QCOMPARE(alarmStr1, alarmStr2);


    //Marked as sent, so no more this time...
    thermostat.alarmLowIsSent();
    QVERIFY(!thermostat.alarmLowTimeToSend());

    thermostat.valueTimeToSend(35.0);          //Less than 40
    QVERIFY(!thermostat.alarmLowTimeToSend()); // So still no

    thermostat.valueTimeToSend(45.0);         //Above 40
    QVERIFY(!thermostat.alarmLowTimeToSend());// No, but reset internally

    thermostat.valueTimeToSend(25.0);         //Less than 40-10
    QVERIFY(thermostat.alarmLowTimeToSend()); //=> alarm

    //Test to disable
    thermostat.setSetpoint(40.0, 0.0);
    thermostat.setAlarmLevels(false, 10.0, false, 10.0);
    thermostat.valueTimeToSend(60.0);
    QVERIFY(!thermostat.alarmLowTimeToSend());
    thermostat.valueTimeToSend(10.0);
    QVERIFY(!thermostat.alarmLowTimeToSend());
    thermostat.valueTimeToSend(60.0);
    QVERIFY(!thermostat.alarmLowTimeToSend());

    //Test to enable
    thermostat.setAlarmLevels(true, 10.0, false, 10.0);
    thermostat.valueTimeToSend(10.0);
    QVERIFY(thermostat.alarmLowTimeToSend());
}



void TestThermostat::test_alarmHighTimeToSend()
{
    Thermostat thermostat(1);

    double value = 60.0;
    double setpoint = 40.0;
    thermostat.setSetpoint(setpoint, 0.0);
    thermostat.setAlarmLevels(true, 10.0, true, 10.0);

    /// @todo test Low and High both depends on a local static variable....
    thermostat.valueTimeToSend(value);

    QCOMPARE(thermostat.alarmHighTimeToSend(), false);

    for(int i=0; i<(FIRST_ALARM_ALLOWED-1); i++)
    {
        thermostat.valueTimeToSend(value);
        if(thermostat.alarmHighTimeToSend())
        {
            qDebug() << "Error: to early for alarmHigh"<< i;
            QFAIL("FAIL");
        }
    }

    //PRINT_DATA();
    QCOMPARE(thermostat.alarmHighTimeToSend(), true);
    thermostat.alarmHighIsSent();

    //Now the alarm is active
    //Let's lower the value until it stops.
    while(value > setpoint-5)
    {
        thermostat.valueTimeToSend(value);
        if(false != thermostat.alarmHighTimeToSend())
        {
            //PRINT_DATA();
            qDebug() << "Already sent, so not more" << value << setpoint;;
            QFAIL("FAIL");
        }
        value--;
    }

    //PRINT_DATA();
    while(!thermostat.alarmHighTimeToSend())
    {
        value++;
        thermostat.valueTimeToSend(value);
    }

    if(value < (setpoint+thermostat.alarmLevelHigh))
    {
        qDebug() << "Alarm triggered to early" << value << setpoint;
        QFAIL("FAIL");
    }

    //PRINT_DATA();
    QVERIFY(thermostat.alarmHighTimeToSend());

    
    QString alarmStr1 = QString("Alarm High ; value=%1 ; alarm=%2 ; setpoint=%3 ; output=000%")
        .arg(value, 0, 'f', 2)
        .arg((setpoint+thermostat.alarmLevelHigh), 0, 'f', 2)
        .arg(setpoint, 0, 'f', 2);

    QString alarmStr2(thermostat.getAlarmHighString());
    //qDebug() << alarmStr1;
    //qDebug() << alarmStr2;
    QCOMPARE(alarmStr1, alarmStr2);


    //Marked as sent, so no more this time...
    thermostat.alarmHighIsSent();
    QVERIFY(!thermostat.alarmHighTimeToSend());

    thermostat.valueTimeToSend(55.0);          //more than 40+10
    QVERIFY(!thermostat.alarmHighTimeToSend()); // So still no

    thermostat.valueTimeToSend(35.0);         //less than  40
    QVERIFY(!thermostat.alarmHighTimeToSend());// No, but reset internally

    thermostat.valueTimeToSend(55.0);         //Less than 40+10
    QVERIFY(thermostat.alarmHighTimeToSend()); //=> alarm

    //Test to disable
    thermostat.setSetpoint(40.0, 0.0);
    thermostat.setAlarmLevels(false, 10.0, false, 10.0);
    thermostat.valueTimeToSend(20.0);
    QVERIFY(!thermostat.alarmHighTimeToSend());
    thermostat.valueTimeToSend(70.0);
    QVERIFY(!thermostat.alarmHighTimeToSend());
    thermostat.valueTimeToSend(20.0);
    QVERIFY(!thermostat.alarmHighTimeToSend());

    //Test to enable
    thermostat.setAlarmLevels(false, 10.0, true, 10.0);
    thermostat.valueTimeToSend(70.0);
    QVERIFY(thermostat.alarmHighTimeToSend());
}





QTEST_MAIN(TestThermostat)
#include "TestThermostat.moc"
