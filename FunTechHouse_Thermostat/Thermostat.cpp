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
    this->setpoint = 60.0;
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
        stageOut ^= (1 << stage);
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
        setStageOut(0, true);
    }
    else
    {
        //Turn off the output
        setStageOut(0, false);
    }
    return true;
}

bool Thermostat::valueTimeToSend(double value)
{
    this->value = value;

    calcOutput();

    /// @todo check diff...
    
    return true;
}



