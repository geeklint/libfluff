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

#include "stdfmtnum.h"

// IF IEEE 754

FluffFloat fluff_float_htof(float num){
	return num;
}

float fluff_float_ftoh(FluffFloat num){
	return num;
}

FluffFloatDouble fluff_float_double_htof(double num){
	return num;
}

double fluff_float_double_ftoh(FluffFloatDouble num){
	return num;
}

// ENDIF IEEE 754

#include <arpa/inet.h>

static inline uint64_t ltob(uint64_t num){
	uint32_t high_part, low_part;

	high_part = htonl((uint32_t)(num >> 32));
	low_part = htonl((uint32_t)(num & 0xFFFFFFFFLL));
	return (((uint64_t)(low_part)) << 32) | high_part;
}

static inline uint64_t btol(uint64_t num){
	uint32_t high_part, low_part;

	high_part = ntohl((uint32_t)(num >> 32));
	low_part = ntohl((uint32_t)(num & 0xFFFFFFFFLL));
	return (((uint64_t)(low_part)) << 32) | high_part;
}

#if defined(__BYTE_ORDER__)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

FluffInt fluff_int_htof(int64_t num){
	return num;
}

int64_t fluff_int_ftoh(FluffInt num){
	return num;
}

#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */

FluffInt fluff_int_htof(int64_t num){
	return ltob(num);
}

int64_t fluff_int_ftoh(FluffInt num){
	return btol(num);
}

#endif /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
#else /* defined(__BYTE_ORDER__) */

FluffInt fluff_int_htof(int64_t num){
	if (1 == htonl(1)){
		return num;
	} else {
		return ltob(num);
	}
}

int64_t fluff_int_ftoh(FluffInt){
	if (1 == htonl(1)){
		return num;
	} else {
		return btol(num);
	}
}

#endif /* defined(__BYTE_ORDER__) */
