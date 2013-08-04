/**
 * @file Thermostat.h
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

#ifndef  __THERMOSTAT_H
#define  __THERMOSTAT_H

#include <stdint.h>

#include "MQTT_Logic.h"

class Thermostat : public MQTT_Logic
{
     private:
         unsigned int stages; ///< How many output stages does this thermostat have?

         double value;     ///< Measured process value, i.e. temperature.
         double setpoint;  ///< Target value
         uint8_t stageOut; ///< Output state for the stages, bit0 is stage0, bit1 is stage1 etc etc.

         bool setStageOut(unsigned int stage, bool activate);
         bool calcOutput();

     public:
         Thermostat(unsigned int stages);
         unsigned int getStageCount();
         bool getStageOut(unsigned int stage);

         void setSetpoint(double setpoint);
         double getSetpoint();

         bool valueTimeToSend(double value);
};

#endif  // __THERMOSTAT_H
