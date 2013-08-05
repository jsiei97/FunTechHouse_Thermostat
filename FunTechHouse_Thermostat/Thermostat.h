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

//Time until next stage kicks in
#define LOW_VALUE_COUNT_MAX 180
#define OUT_STRING_MAX_SIZE 100


class Thermostat : public MQTT_Logic
{
     private:
         unsigned int stages; ///< How many output stages does this thermostat have?

         double value;     ///< Measured process value, i.e. temperature.
         double setpoint;  ///< Target value
         uint8_t stageOut; ///< Output state for the stages, bit0 is stage0, bit1 is stage1 etc etc.
         char* outString;  ///< A reusable string for the output, so we minimize malloc usage.

         double valueSent;     ///< Last value sent to server.
         double setpointSent;  ///< Last setpoint sent to server.
         uint8_t stageOutSent; ///< Last output sent to server.

         double setpointHyst; ///< Must fall with this much before we active again.

         unsigned int lowValueCount; ///< How many times has we been under the setpoint?

         bool setStageOut(unsigned int stage, bool activate);
         void incStageOut();

         bool calcOutput();
         unsigned int getOutValue();

     public:
         Thermostat(unsigned int stages);
         unsigned int getStageCount();
         bool getStageOut(unsigned int stage);

         void setSetpoint(double setpoint, double hysteresis);
         double getSetpoint();

         bool valueTimeToSend(double value);
         char* getValueString();
         void valueIsSent();
};

#endif  // __THERMOSTAT_H
