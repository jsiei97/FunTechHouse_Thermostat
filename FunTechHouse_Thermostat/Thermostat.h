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

/**
 * Time until next stage kicks in
 */
#define LOW_VALUE_COUNT_MAX 180

/**
 * Max size for the internal string buffer
 */
#define OUT_STRING_MAX_SIZE 100

/**
 * If value is the "same" for "cnt" questions, then send anyway.
 *
 * If sleep is 1s (1000ms) and there is 1 question per rotation
 * then we have 600/1s=600s or always send every 10min
 * 1200/1s/60s=20min
 */
#define ALWAYS_SEND_CNT 1200

/**
 * Do not send any alarms at startup wait for the process to start as well.
 */
#define FIRST_ALARM_ALLOWED 600

/**
 * The statemachine for the alarm
 */
typedef enum
{
    ALARM_ACTIVE_SENT = 0, ///< The alarm is triggered, and has been sent to the server.
    ALARM_ACTIVE_NOT_SENT, ///< The alarm is triggered, but has not sent to the server.
    ALARM_NOT_ACTIVE       ///< The alarm is not triggered, all is fine.
} AlarmStates;

/**
 * What output type this thermostat is used.
 */
typedef enum
{
    THERMOSTAT_TYPE_LINEAR = 0, ///< Linear output, 3stages, 001, 011, 111
    THERMOSTAT_TYPE_BIN_CNT     ///< Bin cnt output, 3stages, 001, 010, 011, 100, 101, 110, 111
} ThermostatType;


/**
 * A thermostat with multi stage output.
 *
 * If the value is to low, it activates the output.
 * If the value is to high, it deactives the output.
 * After some time if the value has not rised enought,
 * the second output stage will be actived.
 *
 * It also has two alarm functions that will notify you if the
 * process has failed in some way.
 *
 * @dotfile state_alarm_low.gv The alarm low state machine
 * @dotfile state_alarm_high.gv The alarm high state machine
 */
class Thermostat : public MQTT_Logic
{
     private:
         unsigned int stages; ///< How many output stages does this thermostat have?
         ThermostatType type; ///< What output type to use.

         double value;     ///< Measured process value, i.e. temperature.
         double setpoint;  ///< Target value
         uint8_t stageOut; ///< Output state for the stages, bit0 is stage0, bit1 is stage1 etc etc.
         uint8_t maxOutValue; ///< Out not allowed to be bigger than this, more or less limit out to this.
         char* outString;  ///< A reusable string for the output, so we minimize malloc usage.

         double valueSent;     ///< Last value sent to server.
         double setpointSent;  ///< Last setpoint sent to server.
         uint8_t stageOutSent; ///< Last output sent to server.

         double setpointHyst; ///< Must fall with this much before we active again.

         unsigned int lowValueCount; ///< How many times has we been under the setpoint?

         double valueDiffMax; ///< Value should diff more than this to be sent to the server
         int    valueSendCnt; ///< Always send after "cnt time" even if there is no change

         bool alarmLowActive; ///< Is low alarm active? if false then low alarm is off
         double alarmLevelLow;///< Alarm level, setpoint-alarmLevelLow=>alarm
         AlarmStates alarmLow;///< The low alarm statemachine.

         bool alarmHighActive; ///< Is high alarm active? if false then high alarm is off
         double alarmLevelHigh;///< Alarm level, setpoint+alarmLevelHigh=>alarm
         AlarmStates alarmHigh;//< The high alarm statemachine

         void incStageOut();
         //void decStageOut();
         bool isOutMax();

         unsigned int firstAlarm; ///< Countdown timer so we dont sent the first alarms to early.
         bool allowAlarm();
         bool calcOutput();
         unsigned int getOutValue();

     public:
         Thermostat(unsigned int stages, ThermostatType type);
         unsigned int getStageCount();
         bool getStageOut(unsigned int stage);

         bool setOutMax(uint8_t maxValue);

         void setSetpoint(double setpoint, double hysteresis);
         void setValueDiff(double valueDiffMax);
         void setAlarmLevels(bool activateLowAlarm, double alarmLevelLow,
                 bool activateHighAlarm, double alarmLevelHigh);

         bool  valueTimeToSend(double value);
         char* getValueString();
         void  valueIsSent();

         bool  alarmLowTimeToSend();
         char* getAlarmLowString();
         void  alarmLowIsSent();

         bool  alarmHighTimeToSend();
         char* getAlarmHighString();
         void  alarmHighIsSent();

         //bool  alarmError();
         //char* getAlarmErrorString();
         //void  alarmHighIsSent();
};

#endif  // __THERMOSTAT_H
