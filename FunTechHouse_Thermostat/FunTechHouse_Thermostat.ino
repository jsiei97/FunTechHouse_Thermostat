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

#include "LM35DZ.h"
#include "ValueAvg.h"
#include "TemperatureSensor.h"


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
    thermostat.setOutMax(0x4); // => 4(9kW), later 2(4kW) or 3(6kW)
    thermostat.setAlarmLevels(true, 15.0, true, 10.0); // 60-15=45 60+10=70
    thermostat.setTopic(
            "FunTechHouse/ElPanna_Data",
            "FunTechHouse/ElPanna"
            );

    //Config the first sensor
    sensors[0].setAlarmLevels(false, 25.0, false, 22.0);
    sensors[0].setSensor(TemperatureSensor::LM35DZ, A1);
    sensors[0].setDiffToSend(1.4);
    sensors[0].setTopic(
            "FunTechHouse/GT1-VV-Data",
            "FunTechHouse/GT1-VV"
            );

    //Then configure a second sensor
    sensors[1].setAlarmLevels(false, 25.0, false, 22.0);
    sensors[1].setSensor(TemperatureSensor::LM35DZ, A2);
    sensors[1].setDiffToSend(1.4);
    sensors[1].setTopic(
            "FunTechHouse/GT2-VV-Data",
            "FunTechHouse/GT2-VV"
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


    //Part 1.1 - Update Thermostat with new value and check alarms
    filter.init();
    for( int j=0 ; j<9 ; j++ )
    {
        filter.addValue( LM35DZ::analog11_to_temperature( analogRead(A0) ) );
    }
    temperature = filter.getValue();

    //No sensor connected becomes 109deg,
    //so lets just ignore values higher than 105
    if(temperature <= 105.0 && temperature != 0.0)
    {
        if( thermostat.valueTimeToSend(temperature) )
        {
            if(client.publish(
                        thermostat.getTopicPublish(),
                        thermostat.getValueString()))
            {
                thermostat.valueIsSent();
            }
        }

        if( thermostat.alarmLowTimeToSend() )
        {
            if(client.publish(
                        thermostat.getTopicPublish(),
                        thermostat.getAlarmLowString()))
            {
                thermostat.alarmLowIsSent();
            }
        }

        if( thermostat.alarmHighTimeToSend() )
        {
            if(client.publish(
                        thermostat.getTopicPublish(),
                        thermostat.getAlarmHighString()))
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
        bool readOk = false;

        if( ((int)TemperatureSensor::LM35DZ) == sensors[i].getSensorType() )
        {
            //There is some noice so take a avg on some samples
            //so we don't see the noice as much...
            filter.init();
            for( int j=0 ; j<9 ; j++ )
            {
                filter.addValue( 
                        LM35DZ::analog11_to_temperature(
                            analogRead(
                                sensors[i].getSensorPin()
                                ) 
                            )
                        );
            }
            temperature = filter.getValue();

            //No sensor connected becomes 109deg,
            //so lets just ignore values higher than 105
            if(temperature <= 105.0 && temperature != 0.0)
            {
                readOk = true;
            }
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
                    if(client.publish(
                            sensors[i].getTopicPublish(), 
                            sensors[i].getValueString()))
                    {
                        sensors[i].valueIsSent();
                    }
                }
            }

            /// @todo Fix alarm logic
        }
    }


    delay(1000);
}
