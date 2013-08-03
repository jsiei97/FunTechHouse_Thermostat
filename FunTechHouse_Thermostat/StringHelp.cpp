/**
 * @file StringHelp.cpp
 * @author Johan Simonsson
 * @brief Helper functions
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

#include "StringHelp.h"
#include <stdlib.h>

/**
 * Split a double into the integer and decimal part,
 * since the arduino sprintf cant handle double.
 *
 * Please note that the decimal part will be 2 digits, 
 * so you may need to fill with zero when it is printed.
 * i.e. 4.04 will return 4 and 4, and 5.2 will return 5 and 20.
 *
 * @param value[in] The value to split
 * @param integer[out] The integer part that will be returned
 * @param decimal[out] The decimal part that will be returned
 */
void StringHelp::splitDouble(double value, int* integer, int* decimal)
{
    *integer = (int)(value);
    *decimal = (int)((value-*integer)*100);
    *decimal = abs(*decimal);
}
