/**
 * @file TestMQTT_Logic.cpp
 * @author Johan Simonsson  
 * @brief Testfile for MQTT_Logic.
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

#include "MQTT_Logic.h"

class TestMQTT_Logic : public QObject
{
    Q_OBJECT

    private:
    public:

    private slots:
        void test_setTopic();
        void test_checkTopic();
};


/**
 * Test setTopic and getTopic
 */
void TestMQTT_Logic::test_setTopic()
{
    MQTT_Logic mqttLogic[4];

    QCOMPARE( true, mqttLogic[0].setTopic("in_0", "out_0") );
    QCOMPARE( true, mqttLogic[1].setTopic("in_1", "out_1") );
    QCOMPARE( true, mqttLogic[2].setTopic("in_2", "out_2") );
    QCOMPARE( true, mqttLogic[3].setTopic("in_3", "out_3") );

    QCOMPARE( mqttLogic[0].getTopicSubscribe(), "in_0" );
    QCOMPARE( mqttLogic[0].getTopicPublish(),   "out_0" );

    QCOMPARE( mqttLogic[3].getTopicSubscribe(), "in_3" );
    QCOMPARE( mqttLogic[3].getTopicPublish(),   "out_3" );
}


/**
 * Tests checkTopic
 */
void TestMQTT_Logic::test_checkTopic()
{
    MQTT_Logic mqttLogic;
    mqttLogic.setTopic("in_0", "out_0");

    QCOMPARE(true,  mqttLogic.checkTopicSubscribe("in_0"));
    QCOMPARE(false, mqttLogic.checkTopicSubscribe("bogus"));

    mqttLogic.setTopic("house/party1/data", "out_0");
    QCOMPARE(true,  mqttLogic.checkTopicSubscribe("house/party1/data"));
    QCOMPARE(false, mqttLogic.checkTopicSubscribe("house/party2/data"));
}

QTEST_MAIN(TestMQTT_Logic)
#include "TestMQTT_Logic.moc"
