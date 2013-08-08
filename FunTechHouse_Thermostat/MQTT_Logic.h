/**
 * @file MQTT_Logic.h
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

#ifndef  __MQTT_LOGIC_H
#define  __MQTT_LOGIC_H

class MQTT_Logic
{
    protected:

    private: 
        char* topicIn; ///< MQTT topic for data from the server
        char* topicOut;///< MQTT topic for data to the server

    public:
        MQTT_Logic();

        bool setTopic(char* topicSubscribe, char* topicPublish);
        char* getTopicSubscribe();
        char* getTopicPublish();
        bool checkTopicSubscribe(char* check);

};

#endif  // __MQTT_LOGIC_H 
