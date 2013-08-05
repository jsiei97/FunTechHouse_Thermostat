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

    lowValueCount = 0;
    outString = (char*)malloc(sizeof(char)*(OUT_STRING_MAX_SIZE+1));
};

unsigned int Thermostat::getStageCount()
{
    return stages;
}

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

double Thermostat::getSetpoint()
{
    return setpoint;

}
void Thermostat::setSetpoint(double setpoint, double hysteresis)
{
    this->setpoint = setpoint;
    setpointHyst = hysteresis;
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

bool Thermostat::valueTimeToSend(double value)
{
    bool timeToSend = false;

    this->value = value;

    calcOutput();

    if(value != valueSent)
        timeToSend = true;

    if(setpoint != setpointSent)
        timeToSend = true;

    if(stageOut != stageOutSent)
        timeToSend = true;

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
    //stages
    //stageOut
    //stOut*100/stages=>out

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
