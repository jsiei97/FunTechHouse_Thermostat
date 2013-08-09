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

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x03 };

// The MQTT device name, this must be unique
char project_name[]  = "FunTechHouse_Thermostat";

Thermostat thermostat(3);

PubSubClient client("mosqhub", 1883, callback);

//The stage out relays is connected to:
int gpioStage0  = 2;
int gpioStage1  = 3;
int gpioStage2  = 5;

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
    thermostat.setAlarmLevels(true, 15., true, 10.0);
    thermostat.setTopic(
            "FunTechHouse/Room1/ThermostatData",
            "FunTechHouse/Room1/Thermostat"
            );
}

void setup()
{
    analogReference(EXTERNAL); //3.3V
    pinMode(gpioStage0, OUTPUT);
    pinMode(gpioStage1, OUTPUT);
    pinMode(gpioStage2, OUTPUT);

    //Configure this project.
    configure();

    //Start ethernet, if no ip is given then dhcp is used.
    Ethernet.begin(mac);
    if (client.connect(project_name))
    {
        client.publish( thermostat.getTopicPublish(), "#Hello world" );
        client.subscribe( thermostat.getTopicSubscribe() );
    }
}

void loop()
{
    int reading = analogRead(A0);
    double temperature = reading * 3.30; // 3.3V Aref
    temperature /= 1024.0;               // ADC resolution
    temperature *= 100;                  // 10mV/C (0.01V/C)

    if(false == client.loop())
    {
        client.connect(project_name);
    }
    if(false == client.connected())
    {
        client.connect(project_name);
    }


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


    delay(1000);
}
