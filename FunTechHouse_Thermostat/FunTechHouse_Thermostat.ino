/**
 * @file FunTechHouse_Thermostat.ino
 * @author Johan Simonsson
 * @brief Main file
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

#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"
#include "Thermostat.h"

#include "LVTS.h"
#include "ValueAvg.h"
#include "TemperatureSensor.h"

#define OUT_STR_MAX 100

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x05 };

// The MQTT device name, this must be unique
char project_name[]  = "FunTechHouse_Thermostat";

Thermostat thermostat(3, THERMOSTAT_TYPE_BIN_CNT);
#define SENSOR_CNT 2
TemperatureSensor sensors[SENSOR_CNT];

PubSubClient client("mosqhub", 1883, callback);

//The stage out relays is connected to:
int gpioStage0  = 2;
int gpioStage1  = 5; //Upps built the hw with the gpio in the wrong order.
int gpioStage2  = 3;

void callback(char* topic, byte* payload, unsigned int length)
{
    // handle message arrived
    //client.publish(topic_out, "echo...");
}


void configure()
{
    //Config the thermostat
    thermostat.setSetpoint(60.0, 5.0); //55..60
    thermostat.setValueDiff(1.0);
    thermostat.setOutMax(0x3); // => 4(9kW), later 2(4kW) or 3(6kW)
    thermostat.setAlarmLevels(true, 15.0, true, 10.0); // 60-15=45 60+10=70
    thermostat.setTopic(
            "FunTechHouse/Pannrum/ElPanna_Data",
            "FunTechHouse/Pannrum/ElPanna"
            );

    //Config the first sensor
    sensors[0].setAlarmLevels(false, 25.0, false, 22.0);
    sensors[0].setSensor(TemperatureSensor::LM35DZ, A1);
    sensors[0].setDiffToSend(1.4);
    sensors[0].setTopic(
            "FunTechHouse/Pannrum/GT1-VV_Data",
            "FunTechHouse/Pannrum/GT1-VV"
            );

    //Then configure a second sensor
    sensors[1].setAlarmLevels(false, 25.0, false, 22.0);
    sensors[1].setSensor(TemperatureSensor::LM35DZ, A2);
    sensors[1].setDiffToSend(1.4);
    sensors[1].setTopic(
            "FunTechHouse/Pannrum/GT2-VV_Data",
            "FunTechHouse/Pannrum/GT2-VV"
            );
}

void setup()
{
    //INTERNAL: an built-in reference, equal to 1.1 volts on the ATmega168 or ATmega328
    analogReference(INTERNAL); //1.1V

    pinMode(gpioStage0, OUTPUT);
    pinMode(gpioStage1, OUTPUT);
    pinMode(gpioStage2, OUTPUT);

    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A4, INPUT);

    //Configure this project.
    configure();

    //Start ethernet, if no ip is given then dhcp is used.
    Ethernet.begin(mac);
    if (client.connect(project_name))
    {
        client.publish( thermostat.getTopicPublish(), "#Hello world" );
        client.subscribe( thermostat.getTopicSubscribe() );

        for( int i=0 ; i<SENSOR_CNT; i++ )
        {
            client.publish( sensors[i].getTopicPublish(), "#Hello world" );
            client.subscribe( sensors[i].getTopicSubscribe() );
        }
    }
}

void loop()
{
    if(false == client.loop())
    {
        client.connect(project_name);
    }
    if(false == client.connected())
    {
        client.connect(project_name);
    }

    ValueAvg filter;
    double temperature = 0;
    char str[OUT_STR_MAX];

    //Part 1.1 - Update Thermostat with new value and check alarms
    bool ok = true;
    filter.init();
    for( int j=0 ; j<9 ; j++ )
    {
        bool res = false;
        filter.addValue( LVTS::lm35( analogRead(A0), &res ) );
        if(false == res)
        {
            ok = false;
        }
    }
    temperature = filter.getValue();

    //No sensor connected becomes 109deg,
    //so lets just ignore values higher than 105
    if(ok)
    {
        if( thermostat.valueTimeToSend(temperature) )
        {
            thermostat.getValueString( str, OUT_STR_MAX );
            if(client.publish( thermostat.getTopicPublish(), str))
            {
                thermostat.valueIsSent();
            }
        }

        if( thermostat.alarmLowTimeToSend() )
        {
            thermostat.getAlarmLowString( str, OUT_STR_MAX );
            if(client.publish( thermostat.getTopicPublish(), str) )
            {
                thermostat.alarmLowIsSent();
            }
        }

        if( thermostat.alarmHighTimeToSend() )
        {
            thermostat.getAlarmHighString( str, OUT_STR_MAX );
            if(client.publish( thermostat.getTopicPublish(), str) )
            {
                thermostat.alarmHighIsSent();
            }
        }

        // Part 1.2 - Update the outputs with the latest data.
        if(thermostat.getStageOut(0))
        {
            digitalWrite(gpioStage0, HIGH);
        }
        else
        {
            digitalWrite(gpioStage0, LOW);
        }

        if(thermostat.getStageOut(1))
        {
            digitalWrite(gpioStage1, HIGH);
        }
        else
        {
            digitalWrite(gpioStage1, LOW);
        }

        if(thermostat.getStageOut(2))
        {
            digitalWrite(gpioStage2, HIGH);
        }
        else
        {
            digitalWrite(gpioStage2, LOW);
        }
    }
    else
    {
        //This is bad, we don't have a sensor to play with!
        //All out to Zero
        digitalWrite(gpioStage0, LOW);
        digitalWrite(gpioStage1, LOW);
        digitalWrite(gpioStage2, LOW);
        /// @todo Send a alarm that the sensor is broken!!!
    }

    // Part 2.1 - Loop the misc sensors attached to this device.
    for( int i=0 ; i<SENSOR_CNT; i++ )
    {
        bool readOk = true;

        if( ((int)TemperatureSensor::LM35DZ) == sensors[i].getSensorType() )
        {
            //There is some noice so take a avg on some samples
            //so we don't see the noice as much...
            filter.init();
            for( int j=0 ; j<9 ; j++ )
            {
                bool res = false;
                filter.addValue(
                        LVTS::lm35(
                            analogRead( sensors[i].getSensorPin() ),
                            &res
                            )
                        );
                if(false == res)
                {
                    readOk = false;
                }
            }
            temperature = filter.getValue();
        }

        if(true == readOk)
        {
            if(false == client.connected())
            {
                client.connect(project_name);
            }

            //Check and save the current value
            if( sensors[i].valueTimeToSend(temperature) )
            {
                if(client.connected())
                {
                    sensors[i].getValueString( str, OUT_STR_MAX );
                    if( client.publish( sensors[i].getTopicPublish(), str) )
                    {
                        sensors[i].valueIsSent();
                    }
                }
            }

            if(sensors[i].alarmHighCheck(str, OUT_STR_MAX))
            {
                if( (false == client.connected()) || (false == client.publish(sensors[i].getTopicPublish(), str)) )
                {
                    sensors[i].alarmHighFailed();
                }
            }

            if(sensors[i].alarmLowCheck(str, OUT_STR_MAX))
            {
                if( (false == client.connected()) || (false == client.publish(sensors[i].getTopicPublish(), str)) )
                {
                    sensors[i].alarmLowFailed();
                }
            }
        }
    }


    delay(1000);
}
