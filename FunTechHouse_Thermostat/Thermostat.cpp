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

    lowValueCount = 0;
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
void Thermostat::setSetpoint(double setpoint)
{
    this->setpoint = setpoint;
}


/**
 * Calculate the new output.
 *
 * @return true if ok
 */
bool Thermostat::calcOutput()
{
    if(value < setpoint)
    {
        //Turn on the output
        lowValueCount++;

        if(lowValueCount % LOW_VALUE_COUNT_MAX == 0)
        {
            //Enable next step
            incStageOut();
        }

        setStageOut(0, true);
    }
    else
    {
        //Turn off the output
        lowValueCount = 0;

        //setStageOut(0, false);
        stageOut = 0x0;
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
    /// @todo snprintf("value= setpoint= out=0..100%",...)
    return NULL;
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
