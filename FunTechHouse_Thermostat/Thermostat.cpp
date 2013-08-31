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
Thermostat::Thermostat(unsigned int stageCount, ThermostatType type)
{
    stages = stageCount;
    stageOut = 0;
    this->type = type;

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

    firstAlarm = FIRST_ALARM_ALLOWED;

    alarmLowActive = false;
    alarmLevelLow = 10.0;
    alarmHighActive = false;
    alarmLevelHigh = 10.0;

    alarmLow  = ALARM_NOT_ACTIVE;
    alarmHigh = ALARM_NOT_ACTIVE;

    maxOutValue = ((1 << stages)-1);

    //The delayed off is default not active.
    delayOffCount = 0;
    delayOff      = 0;
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
    if(stageOut < maxOutValue)
    {
        switch ( type )
        {
            case THERMOSTAT_TYPE_LINEAR:
                stageOut <<= 1;
                stageOut |= 0x1;
                break;
            case THERMOSTAT_TYPE_BIN_CNT:
                stageOut++;
                break;
            default :
                break;
        }
    }
}

/**
 * Is the output 100%
 *
 * @return true if output is 100%, false if there is more to give.
 */
bool Thermostat::isOutMax()
{
    if(stageOut >= maxOutValue)
    {
        return true;
    }
    return false;
}

/**
 * Set the maximum allows value for the outputs.
 *
 * Example with THERMOSTAT_TYPE_BIN_CNT with 3 stages connected to a
 * electric heater where stage 1 is 2kW, stage 2 is 4kW and stage3 is 9kW.
 * And this heater can produce 2+4+9=15kW, but it is only connected to fuses that allows 10kW.
 * Then a maxValue at 0x4 (bin 100), will allows it to step throu stages that reprecent
 * 0kW, 2kW, 4kW, 6kW (2+4) and 9kW, but block the higher 11kW (9+2) and higher that would blow the fuse.
 *
 * @param maxValue is the new max value.
 * @return true if ok, false is probably a value bigger that the value spec by the stage count.
 */
bool Thermostat::setOutMax(uint8_t maxValue)
{
    if(maxValue > ((1 << stages)-1))
    {
        return false;
    }

    this->maxOutValue = maxValue;
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
 * Set a delay for turn off when high
 *
 * This will delay the turn off when the value is high, 
 * this is practical if the device has its own high level protection 
 * and must run high for X time. 
 * But this is also dangerous if there is no secondary protection, 
 * use this feature with care. 
 *
 * @param delayOffCount time in seconds between the time the value hits the setpoint and we actually turn off.
 */
void Thermostat::setDelayOff(unsigned int delayOffCount)
{
    this->delayOffCount = delayOffCount;
    this->delayOff      = delayOffCount;
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
        if(value < (setpoint-setpointHyst))
        {
            //We are still really low, let's think about more power!
            lowValueCount++;

            if(lowValueCount % LOW_VALUE_COUNT_MAX == 0)
            {
                //Since we are still under the setpoint,
                //let's active the next step.
                incStageOut();
            }
        }
        else if(value < setpoint)
        {
            //We are still low, but probably going in the right direction (rising)
            //but if the output is not good enought, then we fall under hyst and 
            //get more power and then try again...
        }
        else
        {
            //We are higher than setpoint,
            //time to turn off the output.

            if(delayOff > 0)
            {
                //We are delayed, wait a little more...
                delayOff--;
            }
            else
            {
                //No more delays, time to turn off the outputs!
                delayOff = delayOffCount;

                lowValueCount = 0;
                stageOut = 0x0;
            }
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
bool Thermostat::getValueString(char* data, int size)
{
    int vI, vD;
    int sI, sD;

    StringHelp::splitDouble(value, &vI, &vD);
    StringHelp::splitDouble(setpoint, &sI, &sD);

    int res = snprintf(data, size,
            "value=%d.%02d ; setpoint=%d.%02d ; output=%03d%%",
            vI, vD,
            sI, sD, getOutValue());

    if(res < size)
        return true;
    
    return false;
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

    switch ( type )
    {
        case THERMOSTAT_TYPE_LINEAR:
            while(steps<stages)
            {
                if(!getStageOut(steps))
                {
                    break;
                }
                steps++;
            }
            /// @todo Use maxOutValue;
            out = ((steps*100.0)/stages);
            break;
        case THERMOSTAT_TYPE_BIN_CNT:
            out = (stageOut*100) / maxOutValue;
            break;
        default :
            break;
    }

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
            if( (value < (setpoint-alarmLevelLow)) && isOutMax() )
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
bool Thermostat::getAlarmLowString(char* data, int size)
{
    int vI, vD;
    int sI, sD;
    int aI, aD;

    StringHelp::splitDouble(value, &vI, &vD);
    StringHelp::splitDouble(setpoint, &sI, &sD);
    StringHelp::splitDouble((setpoint-alarmLevelLow), &aI, &aD);

    int res = snprintf(data, size,
            "Alarm: Low ; value=%d.%02d ; alarm=%d.%02d ; setpoint=%d.%02d ; output=%03d%%",
            vI, vD,
            aI, aD,
            sI, sD,
            getOutValue());

    if(res < size)
        return true;
    
    return false;
}

/**
 * Returns a alarm high string that can be sent to the server.
 *
 * This functions must only be called if alarmHighTimeToSend retured true.
 *
 * @return char* with the high alarm string
 */
bool Thermostat::getAlarmHighString(char* data, int size)
{
    int vI, vD;
    int sI, sD;
    int aI, aD;

    StringHelp::splitDouble(value, &vI, &vD);
    StringHelp::splitDouble(setpoint, &sI, &sD);
    StringHelp::splitDouble((setpoint+alarmLevelHigh), &aI, &aD);

    int res = snprintf(data, size,
            "Alarm: High ; value=%d.%02d ; alarm=%d.%02d ; setpoint=%d.%02d ; output=%03d%%",
            vI, vD,
            aI, aD,
            sI, sD,
            getOutValue());

    if(res < size)
        return true;
    
    return false;
}

/**
 * Tell the logic that the alarm low was sucessfully sent to the server so it can be marked as sent.
 */
void Thermostat::alarmLowIsSent()
{
    switch ( alarmLow )
    {
        case ALARM_ACTIVE_SENT:
            break;
        case ALARM_NOT_ACTIVE:
            break;
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
        case ALARM_ACTIVE_SENT:
            break;
        case ALARM_NOT_ACTIVE:
            break;
        case ALARM_ACTIVE_NOT_SENT:
            alarmHigh = ALARM_ACTIVE_SENT;
            break;
    }
}
