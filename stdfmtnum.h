/*
	Copyright 2014 Sky Leonard
	This file is part of libfluff.

    libfluff is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libfluff is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libfluff.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FLUFF_STDFMTNUM_H_
#define FLUFF_STDFMTNUM_H_

#include <stdint.h>

typedef uint32_t FluffFloat;
typedef uint64_t FluffFloatDouble;
typedef uint64_t FluffInt;

FluffFloat fluff_float_htof(float);

float fluff_float_ftoh(FluffFloat);

FluffFloatDouble fluff_float_double_htof(double);

double fluff_float_double_ftoh(FluffFloatDouble);

FluffInt fluff_int_htof(int64_t);

int64_t fluff_int_ftoh(FluffInt);

#endif /* FLUFF_STDFMTNUM_H_ */
