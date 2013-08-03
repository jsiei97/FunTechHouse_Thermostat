/**
 * @file MQTT_Logic.cpp
 * @author Johan Simonsson
 * @brief The MQTT logic with topics for subscribe and publish.
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

#include "MQTT_Logic.h"

/**
 * Default constructur
 */
MQTT_Logic::MQTT_Logic()
{
    topicIn  = NULL;
    topicOut = NULL;
}


/**
 * What mqtt topics this sensor will use.
 *
 * @param topicSubscribe data from the mqtt server
 * @param topicPublish data to the mqtt server
 * @return true if ok
 */
bool MQTT_Logic::setTopic(char* topicSubscribe, char* topicPublish)
{
    int len = strlen(topicSubscribe);
    topicIn = (char*)malloc(len+1);
    memcpy(topicIn , topicSubscribe, len);
    topicIn[len] = '\0';

    len = strlen(topicPublish);
    topicOut = (char*)malloc(len+1);
    memcpy(topicOut , topicPublish, len);
    topicOut[len] = '\0';

    return true;
}

char* MQTT_Logic::getTopicSubscribe()
{
    return topicIn;
}

char* MQTT_Logic::getTopicPublish()
{
    return topicOut;
}

bool MQTT_Logic::checkTopicSubscribe(char* check)
{
    bool res = false;

    if(0 == strcmp(check,topicIn))
    {
        res = true;
    }

    return res;
}
