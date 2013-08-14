#include <QtCore>
#include <QtTest>

#include "TemperatureSensor.h"

class TestTemperatureSensor : public QObject
{
    Q_OBJECT

    private:
    public:

    private slots:
        //void test_add_data();
        //void test_add();

        void test_valueTimeToSend();
        void test_alarm();
        void test_offset();
        void test_offset_data();
        void test_setSensor();
        void test_setTopic();
        void test_checkTopic();
};

/*
void TestTemperatureSensor::test_add_data()
{
    QTest::addColumn<int>("num1");
    QTest::addColumn<int>("num2");
    QTest::addColumn<int>("sum");

    QTest::newRow("add test 01") << 1 << 1 << 2;
    QTest::newRow("add test 02") << 2 << 2 << 4;
};


void TestTemperatureSensor::test_add()
{
    QFETCH(int, num1);
    QFETCH(int, num2);
    QFETCH(int, sum);


    //Calc calc;
    //QCOMPARE(calc.add(num1,num2), sum);
}
*/

/**
 * Test that we do not send to many duplicate data.
 */
void TestTemperatureSensor::test_valueTimeToSend()
{
    TemperatureSensor sensor;

    bool res = sensor.valueTimeToSend(10.0);
    QCOMPARE(res, true); //Always send the first time...
    for(int i=1; i<=ALWAYS_SEND_CNT; i++)
    {
        res = sensor.valueTimeToSend(10.0);
        if(true == res)
        {
            qDebug() << "Error we should not send now" << i;
            QFAIL("Error valueSendCnt wrong");
        }
    }

    sensor.setDiffToSend(0.8);

    //Then we shall send
    QCOMPARE(sensor.valueTimeToSend(10.0), true);
    //And we are back in the loop
    QCOMPARE(sensor.valueTimeToSend(10.0), false);

    //Now check with a +diff
    QCOMPARE(sensor.valueTimeToSend(10.9), true);  //diff > 0.8 => OK
    QCOMPARE(sensor.valueTimeToSend(11.0), false); //diff < 0.8 => Not ok
    QCOMPARE(sensor.valueTimeToSend(12.3), true);  //diff 12.3-10.9=1.4 > 0.8 => OK

    //Now check with a -diff
    QCOMPARE(sensor.valueTimeToSend(10.3), true);  //diff > 0.8 => OK
    QCOMPARE(sensor.valueTimeToSend(10.0), false); //diff < 0.8 => Not ok
    QCOMPARE(sensor.valueTimeToSend( 9.4), true);  //diff 10.3-9.4=0.9 > 0.8 => OK
}


/**
 * Test the alarm functions 
 * setAlarmLevels, alarmHighCheck, alarmLowCheck, alarmHighFailed, alarmLowFailed
 */
void TestTemperatureSensor::test_alarm()
{
    TemperatureSensor sensor;

    //No alarms, never return true
    sensor.setAlarmLevels(false, 25.0, false, 20.0);

    char str[40];

    sensor.valueTimeToSend(15.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false);
    sensor.valueTimeToSend(22.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false);
    sensor.valueTimeToSend(35.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false);

    sensor.valueTimeToSend(15.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false);
    sensor.valueTimeToSend(22.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false);
    sensor.valueTimeToSend(25.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false);


    //High alarm active, but not low alarm.
    sensor.setAlarmLevels(true, 25.0, false, 25.0);

    sensor.valueTimeToSend(15.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false);
    sensor.valueTimeToSend(26.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), true);
    sensor.valueTimeToSend(27.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false); //Only 1 alarm!
    sensor.alarmHighFailed();
    sensor.valueTimeToSend(26.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), true);
    sensor.valueTimeToSend(26.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false);
    sensor.valueTimeToSend(24.9);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false); //Check that the hyst is working
    sensor.valueTimeToSend(25.1);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false);
    sensor.valueTimeToSend(23.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false); //Reset alarm, by dropping under the level
    sensor.valueTimeToSend(26.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), true); // Then we get a new alarm

    //Low is still deactivated
    sensor.valueTimeToSend(20.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false);
    sensor.valueTimeToSend(30.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false);

    //Low alarm active, but no high alarm
    sensor.setAlarmLevels(false, 25.0, true, 20.0);

    sensor.valueTimeToSend(30.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false); //Higher no alarm
    sensor.valueTimeToSend(18.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), true); //lower -> alarm
    sensor.valueTimeToSend(18.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false); //but only once
    sensor.valueTimeToSend(20.1);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false); //Check that hyst is working
    sensor.valueTimeToSend(19.9);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false); 
    sensor.valueTimeToSend(22.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false); //Reset by going over
    sensor.valueTimeToSend(18.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), true);  //Then we get a new alarm
    QCOMPARE(sensor.alarmLowCheck(str, 40), false); //but only once

    sensor.alarmLowFailed(); //Send failed so we can mark that the alarm was not sent
    sensor.valueTimeToSend(18.0);
    QCOMPARE(sensor.alarmLowCheck(str, 40), true); // Then we get a new alarm
    QCOMPARE(sensor.alarmLowCheck(str, 40), false); //but only once

    //High is deactivated
    sensor.valueTimeToSend(24.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false);
    sensor.valueTimeToSend(26.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false);

    // Both alarms active
    sensor.setAlarmLevels(true, 22.0, true, 20.0);

    sensor.valueTimeToSend(21.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false);

    sensor.valueTimeToSend(23.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), true);
    QCOMPARE(sensor.alarmLowCheck(str, 40), false);

    sensor.valueTimeToSend(19.0);
    QCOMPARE(sensor.alarmHighCheck(str, 40), false);
    QCOMPARE(sensor.alarmLowCheck(str, 40), true);
}


void TestTemperatureSensor::test_offset_data()
{
    QTest::addColumn<double>("value");
    QTest::addColumn<double>("offset");
    QTest::addColumn<double>("result");

    QTest::newRow("No offset")   << 15.0 <<  0.0 << 15.0;
    QTest::newRow("Offset +1")   << 10.0 <<  1.0 << 11.0;
    QTest::newRow("Offset -1")   << 10.0 << -1.0 <<  9.0;
    QTest::newRow("Offset +9.5") << 10.0 <<  9.5 << 19.5;
    QTest::newRow("Offset -9.5") << 10.0 << -9.5 <<  0.5;
}
/**
 * Check that the offset is correct
 */
void TestTemperatureSensor::test_offset()
{
    QFETCH(double, value);
    QFETCH(double, offset);
    QFETCH(double, result);

    TemperatureSensor sensor;

    sensor.setValueOffset(offset);
    sensor.valueTimeToSend(value);
    
    QCOMPARE(sensor.valueWork, result);
}

/**
 * Test the set sensor type and what pin is used
 */
void TestTemperatureSensor::test_setSensor()
{
    TemperatureSensor sensors[3];

    for( int i=0 ; i<3 ; i++ )
    {
        QCOMPARE(sensors[i].getSensorType(), (int)Sensor::NO_SENSOR);
    }

    sensors[1].setSensor(Sensor::LM35DZ, 10);

    QCOMPARE(sensors[0].getSensorType(), (int)Sensor::NO_SENSOR);
    QCOMPARE(sensors[0].getSensorPin(), 0);

    QCOMPARE(sensors[1].getSensorType(), (int)Sensor::LM35DZ);
    QCOMPARE(sensors[1].getSensorPin(), 10);

    QCOMPARE(sensors[2].getSensorType(), (int)Sensor::NO_SENSOR);
    QCOMPARE(sensors[2].getSensorPin(), 0);
}

void TestTemperatureSensor::test_setTopic()
{
    TemperatureSensor sensors[4];

    QCOMPARE( true, sensors[0].setTopic("in_0", "out_0") );
    QCOMPARE( true, sensors[1].setTopic("in_1", "out_1") );
    QCOMPARE( true, sensors[2].setTopic("in_2", "out_2") );
    QCOMPARE( true, sensors[3].setTopic("in_3", "out_3") );

    QCOMPARE( sensors[0].getTopicSubscribe(), "in_0" );
    QCOMPARE( sensors[0].getTopicPublish(),   "out_0" );

    QCOMPARE( sensors[3].getTopicSubscribe(), "in_3" );
    QCOMPARE( sensors[3].getTopicPublish(),   "out_3" );
}


void TestTemperatureSensor::test_checkTopic()
{
    TemperatureSensor sensor;
    sensor.setTopic("in_0", "out_0");

    QCOMPARE(true,  sensor.checkTopicSubscribe("in_0"));
    QCOMPARE(false, sensor.checkTopicSubscribe("bogus"));
    

    sensor.setTopic("house/party1/data", "out_0");
    QCOMPARE(true,  sensor.checkTopicSubscribe("house/party1/data"));
    QCOMPARE(false, sensor.checkTopicSubscribe("house/party2/data"));
}

QTEST_MAIN(TestTemperatureSensor)
#include "TestTemperatureSensor.moc"
