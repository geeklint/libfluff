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
typedef uint64_t FluffDouble;
typedef uint16_t FluffShort;
typedef uint32_t FluffInt;
typedef uint64_t FluffLong;
typedef uint16_t FluffUShort;
typedef uint32_t FluffUInt;
typedef uint64_t FluffULong;

FluffFloat fluff_float_htof(float);
float fluff_float_ftoh(FluffFloat);

FluffDouble fluff_double_htof(double);
double fluff_double_ftoh(FluffDouble);

FluffShort fluff_short_htof(int16_t);
int16_t fluff_short_ftoh(FluffShort);

FluffInt fluff_int_htof(int32_t);
int32_t fluff_int_ftoh(FluffInt);

FluffLong fluff_long_htof(int64_t);
int64_t fluff_long_ftoh(FluffLong);

FluffUShort fluff_ushort_htof(uint16_t);
uint16_t fluff_ushort_ftoh(FluffUShort);

FluffUInt fluff_uint_htof(uint32_t);
uint32_t fluff_uint_ftoh(FluffUInt);

FluffULong fluff_ulong_htof(uint64_t);
uint64_t fluff_ulong_ftoh(FluffULong);

#endif /* FLUFF_STDFMTNUM_H_ */
