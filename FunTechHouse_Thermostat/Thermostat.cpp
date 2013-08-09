/**
 * @file Thermostat.cpp
 * @author Johan Simonsson
 * @brief A multi stage thermostat implementation
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

#include "Thermostat.h"
#include "StringHelp.h"

/**
 * The default constructor.
 *
 * @param stageCount how many output stages to use
 */
Thermostat::Thermostat(unsigned int stageCount)
{
    stages = stageCount;
    stageOut = 0;

    //Some defaults.
    value = 0;
    setpoint = 60.0;

    valueSent    = 0;
    setpointSent = 0;
    stageOutSent = 0;
    setpointHyst = 5;

    valueDiffMax = 0.8;
    valueSendCnt = 0;

    lowValueCount = 0;
    outString = (char*)malloc(sizeof(char)*(OUT_STRING_MAX_SIZE+1));

    firstAlarm = FIRST_ALARM_ALLOWED;

    alarmLowActive = false;
    alarmLevelLow = 10.0;
    alarmHighActive = false;
    alarmLevelHigh = 10.0;

    alarmLow  = ALARM_NOT_ACTIVE;
    alarmHigh = ALARM_NOT_ACTIVE;
};

/**
 * How many output stages is used by this thermostat?
 *
 * @return unsigned int valid values goes from 1..n, 0 is valid but pointless.
 */
unsigned int Thermostat::getStageCount()
{
    return stages;
}

/**
 * If this state is active or not?
 *
 * @param stage the output stage number from 0..n
 * @return true if active and false if not active.
 */
bool Thermostat::getStageOut(unsigned int stage)
{
    if(stage >= stages)
        return false;

    uint8_t mask = 1 << stage;
    return bool(stageOut & mask);
}

/**
 * Enable the next step and keep the old steps active.
 */
void Thermostat::incStageOut()
{
    stageOut <<= 1;
    stageOut |= 0x1;

    /// @todo Do not inc more than max stages
}

/**
 * Change state of any given output stage.
 * Please note that automation will not stop just becaurse the output has changed, 
 * so the output value will be overritten when the system needs it to.
 *
 * @param stage the stage to change
 * @param activate true will turn the output on, false will turn it off.
 */
bool Thermostat::setStageOut(unsigned int stage, bool activate)
{
    if(stage >= stages)
        return false;

    if(activate)
    {
        stageOut |= (1 << stage);
    }
    else
    {
        uint8_t mask = 0xFF;
        mask ^= (1 << stage);
        stageOut &= mask;
    }

    return true;
}

/**
 * Setpoint to work with.
 *
 * Please note that there is a deadzone
 * between setpoint and setpoint-hysteresis.
 *
 * The hysteresis value will prevent a lot of fast output toggle
 * that will burn high power devices and relays.
 *
 * @param setpoint the target value
 * @param hysteresis value must fall lower that setpoint-hysteresis for it to activated.
 */
void Thermostat::setSetpoint(double setpoint, double hysteresis)
{
    this->setpoint = setpoint;
    setpointHyst = hysteresis;
}

/**
 * How much must the value diff from last sent value before it is time to send again?
 *
 * If valueDiffMax is 2.0 and last value sent to server was 20.0, 
 * then the new value must be higher than 22 (20+2) or lower than 18 (20-2)
 * before we send a new value.
 *
 * This will minimize the amount of duplicated data on the server.
 *
 * @param valueDiffMax diff, i.e. 2 will send if 
 */
void Thermostat::setValueDiff(double valueDiffMax)
{
    this->valueDiffMax = valueDiffMax;
}

/**
 * Shall the alarm be active and what level should be used.
 *
 * Please note that the levels is relative to the setpoint.
 * Values 10 and 10, and a setpoint at 50 will give alarm levels at 40 (50-10) and 60 (50+10).
 *
 * Please note that it is a good idea to put alarmLevelLow under setpoint-hyst, 
 * since that is the normal "low point" for the system.
 *
 * @param activateLowAlarm true to activate low alarm
 * @param alarmLevelLow how much lower than setpoint shall the level be?
 * @param activateHighAlarm true to active high alarm
 * @param alarmLevelHigh how much higher than the setpoint shall the level be?
 */
void Thermostat::setAlarmLevels(
        bool activateLowAlarm, double alarmLevelLow, 
        bool activateHighAlarm, double alarmLevelHigh)
{
    alarmLowActive = activateLowAlarm;
    this->alarmLevelLow = alarmLevelLow;

    alarmHighActive = activateHighAlarm;
    this->alarmLevelHigh = alarmLevelHigh;
}

/**
 * Calculate the new output.
 *
 * @return true if ok
 */
bool Thermostat::calcOutput()
{
    if(0 == stageOut)
    {
        //We are in turned off mode and waiting for temperature to drop.
        if(value < (setpoint-setpointHyst))
        {
            //Value is lover than hyst, time to turn on.
            incStageOut();

            //Reset the counter so we get a correct count the second time.
            lowValueCount = 0;
        }
    }
    else
    {
        //We are turned on and waiting for temperature to rise.
        lowValueCount++;

        if(value < setpoint)
        {
            //We are still low
            if(lowValueCount % LOW_VALUE_COUNT_MAX == 0)
            {
                //Since we are still under the setpoint,
                //let's active the next step.
                incStageOut();
            }
        }
        else
        {
            //We are higher than setpoint,
            //time to turn off the output.

            lowValueCount = 0;
            //setStageOut(0, false);
            stageOut = 0x0;
        }
    }
    return true;
}

/**
 * Shall we send data to the server?
 *
 * Please note that the value entered here, 
 * will trigger the calculation of outputs and alarms.
 *
 * @param value the new value used to calculate output
 * @return bool true if there is data to send, false if there is only old data.
 */
bool Thermostat::valueTimeToSend(double value)
{
    bool timeToSend = false;

    this->value = value;

    calcOutput();

    if(0 >= valueSendCnt)
        timeToSend = true;

    double diff = value-valueSent;
    if( diff > valueDiffMax || -diff > valueDiffMax )
        timeToSend = true;

    if(setpoint != setpointSent)
        timeToSend = true;

    if(stageOut != stageOutSent)
        timeToSend = true;

    valueSendCnt--;
    return timeToSend;
}

/**
 * If it is time to send data,
 * this functions prepares the string that should be sent to the server.
 *
 * @return char* to the string
 */
char* Thermostat::getValueString()
{
    int vI, vD;
    int sI, sD;

    StringHelp::splitDouble(value, &vI, &vD);
    StringHelp::splitDouble(setpoint, &sI, &sD);

    snprintf(outString, OUT_STRING_MAX_SIZE,
            "value=%d.%02d ; setpoint=%d.%02d ; output=%03d%%",
            vI, vD,
            sI, sD, getOutValue());
    return outString;
}

/**
 * If we could send the package to the server,
 * then call this function since that will reset the internal counters.
 */
void Thermostat::valueIsSent()
{
    valueSendCnt = ALWAYS_SEND_CNT;

    valueSent    = value;
    setpointSent = setpoint;
    stageOutSent = stageOut;
}

/**
 * Convert the output to a human readable procent number (0..100%)
 *
 * @return number between 0 and 100, where 100 is max.
 */
unsigned int Thermostat::getOutValue()
{
    unsigned int out = 0;
    unsigned int steps = 0;

    while(steps<stages)
    {
        if(!getStageOut(steps))
        {
            break;
        }
        steps++;
    }

    out = ((steps*100.0)/stages);

    return out;
}

/**
 * Delay when we allow the first alarm to be activated, 
 * so the controlled system has time to init.
 *
 * @return true when we allow alarms to be sent
 */
bool Thermostat::allowAlarm()
{
    if(firstAlarm != 0)
    {
        firstAlarm--;
        return false;
    }
    return true;
}

/**
 * Is there a low alarm that should be sent to the server?
 *
 * @return true if there is a alarm, false if all is fine.
 */
bool Thermostat::alarmLowTimeToSend()
{
    if(!allowAlarm())
        return false;

    if(!alarmLowActive)
        return false;

    bool status = false;

    switch ( alarmLow )
    {
        case ALARM_NOT_ACTIVE:
            if( (value < (setpoint-alarmLevelLow)) && (getStageOut(stages-1)) )
            {
                alarmLow = ALARM_ACTIVE_NOT_SENT;
                status = true;
            }
            break;
        case ALARM_ACTIVE_SENT:
            if( value > setpoint )
            {
                alarmLow = ALARM_NOT_ACTIVE;
            }
            break;
        case ALARM_ACTIVE_NOT_SENT:
            if( value > setpoint )
            {
                alarmLow = ALARM_NOT_ACTIVE;
            }
            else
            {
                status = true;
            }
            break;
    }
    return status;
}

/**
 * Is there a high alarm that should be sent to the server?
 *
 * @return true if there is a alarm, false if all is fine.
 */
bool Thermostat::alarmHighTimeToSend()
{
    if(!allowAlarm())
        return false;

    if(!alarmHighActive)
        return false;

    bool status = false;

    switch ( alarmHigh )
    {
        case ALARM_NOT_ACTIVE:
            //if( (value > (setpoint+alarmLevelLow)) && (!getStageOut(0)) )
            if( (value > (setpoint+alarmLevelLow)) )
            {
                alarmHigh = ALARM_ACTIVE_NOT_SENT;
                status = true;
            }
            break;
        case ALARM_ACTIVE_SENT:
            if( value < setpoint )
            {
                alarmHigh = ALARM_NOT_ACTIVE;
            }
            break;
        case ALARM_ACTIVE_NOT_SENT:
            if( value < setpoint )
            {
                alarmHigh = ALARM_NOT_ACTIVE;
            }
            else
            {
                status = true;
            }
            break;
    }
    return status;
}

/**
 * Returns a alarm low string that can be sent 
 * to the server.
 * This functions must only be called if alarmLowTimeToSend retured true.
 *
 * @return char* with the low alarm string
 */
char* Thermostat::getAlarmLowString()
{
    int vI, vD;
    int sI, sD;
    int aI, aD;

    StringHelp::splitDouble(value, &vI, &vD);
    StringHelp::splitDouble(setpoint, &sI, &sD);
    StringHelp::splitDouble((setpoint-alarmLevelLow), &aI, &aD);

    snprintf(outString, OUT_STRING_MAX_SIZE,
            "Alarm Low ; value=%d.%02d ; alarm=%d.%02d ; setpoint=%d.%02d ; output=%03d%%",
            vI, vD,
            aI, aD,
            sI, sD, 
            getOutValue());
    return outString;
}

/**
 * Returns a alarm high string that can be sent to the server.
 *
 * This functions must only be called if alarmHighTimeToSend retured true.
 *
 * @return char* with the high alarm string
 */
char* Thermostat::getAlarmHighString()
{
    int vI, vD;
    int sI, sD;
    int aI, aD;

    StringHelp::splitDouble(value, &vI, &vD);
    StringHelp::splitDouble(setpoint, &sI, &sD);
    StringHelp::splitDouble((setpoint+alarmLevelHigh), &aI, &aD);

    snprintf(outString, OUT_STRING_MAX_SIZE,
            "Alarm High ; value=%d.%02d ; alarm=%d.%02d ; setpoint=%d.%02d ; output=%03d%%",
            vI, vD,
            aI, aD,
            sI, sD, 
            getOutValue());
    return outString;
}

/**
 * Tell the logic that the alarm low was sucessfully sent to the server so it can be marked as sent.
 */
void Thermostat::alarmLowIsSent()
{
    switch ( alarmLow )
    {
        case ALARM_ACTIVE_NOT_SENT:
            alarmLow = ALARM_ACTIVE_SENT;
            break;
    }
}

/**
 * Tell the logic that the alarm high was sucessfully sent to the server so it can be marked as sent.
 */
void Thermostat::alarmHighIsSent()
{
    switch ( alarmHigh )
    {
        case ALARM_ACTIVE_NOT_SENT:
            alarmHigh = ALARM_ACTIVE_SENT;
            break;
    }
}
