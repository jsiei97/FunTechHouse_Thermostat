/**
 * @file LVTS.cpp
 * @author Johan Simonsson
 * @brief Low Voltage Temperature Sensor Class
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

#include "LVTS.h"

/**
 * LM35 temperature sensor.
 *
 * analogReference(INTERNAL); should be used.
 *
 * @param reading from analogRead, (0..1023).
 * @param ok becomes true if ok, false if fail.
 * @return temperature in degC
 */
double LVTS::lm35(int reading, bool *ok)
{
    double aref = 1.10; // Internal 1.1V ref

    double temperature = reading * aref;
    temperature /= 1024.0;               // ADC resolution
    temperature *= 100;                  // 10mV/C (0.01V/C)

    //Datasheet tells us
    //-"Rated for full −55 ̊ to +150 ̊C range"
    //But with Aref 1.1 we only get values 
    //from 0V..1.05V => 0degC..105degC
    if( temperature <= 105.0 && temperature >= 0.0 )
    {
        *ok=true;
        return temperature;
    }

    *ok=false;
    return 0.0;
}

/**
 * LM34 temperature sensor.
 *
 * analogReference(INTERNAL); should be used.
 *
 * @param reading from analogRead, (0..1023).
 * @param ok becomes true if ok, false if fail.
 * @return temperature in degC
 */
double LVTS::lm34(int reading, bool *ok)
{
    double aref = 1.10; // Internal 1.1V ref

    double temperature = reading * aref;
    temperature /= 1024.0;               // ADC resolution
    temperature *= 100;                  // 10mV/C (0.01V/F)

    temperature = F2C(temperature);

    //With Aref 1.1 we get values 
    //  0degF .. 105degF
    //-17degC ..  40degC
    if( temperature <= 40 && temperature >= -17)
    {
        *ok=true;
        return temperature;
    }

    *ok=false;
    return 0.0;
}

/**
 * Convert Fahrenheit to Celcius
 *
 * @param degC value in Celcius
 * @return value in Fahrenheit
 */
double LVTS::F2C(double degC)
{
    degC -= 32;
    degC *= 5;
    degC /= 9;
    return degC; //is now F
}
