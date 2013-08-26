/**
 * @file LM35DZ.cpp
 * @author Johan Simonsson
 * @brief Helper for the LM35DZ sensor
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

#include "LM35DZ.h"

/**
 * Converts a analogRead value into a temperature (Celsius)
 * analogReference(EXTERNAL); should be used and Aref connected to 3.3V.
 *
 * @param reading from analogRead
 * @return temperature in degrees celsius
 */
double LM35DZ::analog33_to_temperature(int reading)
{
    double temperature = reading * 3.30; // 3.3V Aref
    temperature /= 1024.0;               // ADC resolution
    temperature *= 100;                  // 10mV/C (0.01V/C)

    //Datasheet tells us
    //-"Rated for full −55 ̊ to +150 ̊C range"
    if( temperature <= 150 && temperature >= -55)
    {
        return temperature;
    }
    return 0.0;
}

/**
 * Converts a analogRead value into a temperature (Celsius)
 * analogReference(INTERNAL); should be used.
 *
 * @param reading from analogRead
 * @return temperature in degrees celsius
 */
double LM35DZ::analog11_to_temperature(int reading)
{
    double temperature = reading * 1.10; // Internal 1.1V ref
    temperature /= 1024.0;               // ADC resolution
    temperature *= 100;                  // 10mV/C (0.01V/C)

    //Datasheet tells us
    //-"Rated for full −55 ̊ to +150 ̊C range"
    if( temperature <= 150 && temperature >= -55)
    {
        return temperature;
    }
    return 0.0;
}
