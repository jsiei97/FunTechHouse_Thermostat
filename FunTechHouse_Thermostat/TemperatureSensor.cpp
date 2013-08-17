/**
 * @file TemperatureSensor.cpp
 * @author Johan Simonsson
 * @brief A temperature sensor class with alarm logic
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Sensor.h"
#include "TemperatureSensor.h"
#include "StringHelp.h"

/**
 * Default constructur,
 * please note that all alarm are disabled.
 */
TemperatureSensor::TemperatureSensor()
{
    //Some default values
    valueWork = 0.0;
    valueSent = 0.0;
    valueDiffMax = 0.8;
    valueSendCnt = 0;
    valueOffset = 0;

    alarmHigh = 25.0;
    alarmHighActive = false;
    alarmHighSent = false;

    alarmLow = 20.0;
    alarmLowActive = false;
    alarmLowSent = false;

    alarmHyst = 1.0;

    valueSendCnt = ALWAYS_SEND_CNT;
}

/**
 * Is it time to send a new value to the server,
 * this is triggered either on change or timeout.
 *
 * @param value The new temperature
 * @return true if it is time to send
 */
bool TemperatureSensor::valueTimeToSend(double value)
{
    valueWork = value+valueOffset;

    //Timeout lets send anyway
    if(0 == valueSendCnt)
    {
        return true;
    }

    double diff = valueWork-valueSent;
    if( diff > valueDiffMax || -diff > valueDiffMax )
    {
        return true;
    }

    valueSendCnt--;
    return false;
}

void TemperatureSensor::valueIsSent()
{
    valueSendCnt = ALWAYS_SEND_CNT;
    valueSent = valueWork;
}

/**
 * Activate high and low alarm.
 *
 * @param activeHigh true will active the high alarm
 * @param high the high alarm level
 * @param activeLow true will active the low alarm
 * @param low the low alarm level
 */
void TemperatureSensor::setAlarmLevels(bool activeHigh, double high, bool activeLow, double low)
{
    alarmHighActive = activeHigh;
    alarmHigh = high;
    alarmHighSent = false;

    alarmLowActive = activeLow;
    alarmLow = low;
    alarmLowSent = false;
}


/**
 * How much must the value change before we send it?
 *
 * @param value if value is set to X, then we send directly if the messured value diff more than X from the last sent value.
 */
void TemperatureSensor::setDiffToSend(double value)
{
    valueDiffMax = value;
}

/**
 * If a sensor has a static measurment error, 
 * this offset value can be added to correct such a error.
 *
 * @param value the offset value that will be added
 */
void TemperatureSensor::setValueOffset(double value)
{
    valueOffset = value;
}

bool TemperatureSensor::alarmHighCheck(char* responce, int maxSize)
{
    bool sendAlarm = false;
    if(valueWork > alarmHigh)
    {
        if(alarmHighActive && !alarmHighSent)
        {
            alarmHighSent = true;

            int integerPart = 0;
            int decimalPart = 0;
            StringHelp::splitDouble(valueWork, &integerPart, &decimalPart);

            int intAlarm = 0;
            int decAlarm = 0;
            StringHelp::splitDouble(alarmHigh, &intAlarm, &decAlarm);

            snprintf(responce, maxSize, "Alarm: High temperature=%d.%d level=%d.%d",
                    integerPart, decimalPart, intAlarm, decAlarm);
            sendAlarm = true;
        }
    }
    else if( valueWork < (alarmHigh-alarmHyst ) )
    {
        alarmHighSent = false;
    }
    return sendAlarm;
}

bool TemperatureSensor::alarmLowCheck(char* responce, int maxSize)
{
    bool sendAlarm = false;
    if(valueWork < alarmLow)
    {
        if(alarmLowActive && !alarmLowSent)
        {
            alarmLowSent = true;

            int integerPart = 0;
            int decimalPart = 0;
            StringHelp::splitDouble(valueWork, &integerPart, &decimalPart);

            int intAlarm = 0;
            int decAlarm = 0;
            StringHelp::splitDouble(alarmLow, &intAlarm, &decAlarm);

            snprintf(responce, maxSize, "Alarm: Low temperature=%d.%d level=%d.%d",
                    integerPart, decimalPart, intAlarm, decAlarm);
            sendAlarm = true;
        }
    }
    else if( valueWork > (alarmLow+alarmHyst) )
    {
        alarmLowSent = false;
    }

    return sendAlarm;
}

/**
 * Tell the logic that we did not send that alarm.
 */
void TemperatureSensor::alarmHighFailed()
{
    alarmHighSent = false;
}

/**
 * Tell the logic that we did not send that alarm.
 */
void TemperatureSensor::alarmLowFailed()
{
    alarmLowSent = false;
}


