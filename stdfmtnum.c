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

#include <arpa/inet.h>

#define ltob_16 htons
#define btol_16 ntohs
#define ltob_32 htonl
#define btol_32 ntohl

static inline uint64_t ltob_64(uint64_t num){
	uint32_t high_part, low_part;

	high_part = htonl((uint32_t)(num >> 32));
	low_part = htonl((uint32_t)(num & 0xFFFFFFFFLL));
	return (((uint64_t)(low_part)) << 32) | high_part;
}

static inline uint64_t btol_64(uint64_t num){
	uint32_t high_part, low_part;

	high_part = ntohl((uint32_t)(num >> 32));
	low_part = ntohl((uint32_t)(num & 0xFFFFFFFFLL));
	return (((uint64_t)(low_part)) << 32) | high_part;
}

// IF IEEE 754

FluffFloat fluff_float_htof(float num){
	union {
		FluffFloat f;
		float num;
	} join;

	join.num = num;
	return ltob_32(join.f);
}

float fluff_float_ftoh(FluffFloat num){
	union {
		float f;
		FluffFloat num;
	} join;

	join.num = btol_32(num);
	return join.f;
}

FluffDouble fluff_double_htof(double num){
	union {
		FluffDouble f;
		double num;
	} join;

	join.num = num;
	return ltob_64(join.f);
}

double fluff_double_ftoh(FluffDouble num){
	union {
		double f;
		FluffDouble num;
	} join;

	join.num = btol_64(num);
	return join.f;
}

// ENDIF IEEE 754

#if defined(__BYTE_ORDER__)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

FluffShort fluff_short_htof(int16_t num){
	return num;
}

int16_t fluff_short_ftoh(FluffShort, num){
	return num;
}

FluffInt fluff_int_htof(int32_t num){
	return num;
}

int32_t fluff_int_ftoh(FluffInt num){
	return num;
}

FluffLong fluff_long_htof(int64_t num){
	return num;
}

int64_t fluff_long_ftoh(FluffLong num){
	return num;
}

FluffUShort fluff_ushort_htof(uint16_t num){
	return num;
}

uint16_t fluff_ushort_ftoh(FluffUShort num){
	return num;
}

FluffUInt fluff_uint_htof(uint32_t num){
	return num;
}

uint32_t fluff_uint_ftoh(FluffUInt num){
	return num;
}

#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */

FluffShort fluff_short_htof(int16_t num){
	return ltob_16(num);
}

int16_t fluff_short_ftoh(FluffShort num){
	return btol_16(num);
}

FluffInt fluff_int_htof(int32_t num){
	return ltob_32(num);
}

int32_t fluff_int_ftoh(FluffInt num){
	return btol_32(num);
}

FluffLong fluff_long_htof(int64_t num){
	return ltob_64(num);
}

int64_t fluff_long_ftoh(FluffLong num){
	return btol_64(num);
}

FluffUShort fluff_ushort_htof(uint16_t num){
	return ltob_16(num);
}

uint16_t fluff_ushort_ftoh(FluffUShort num){
	return btol_16(num);
}

FluffUInt fluff_uint_htof(uint32_t num){
	return ltob_32(num);
}

uint32_t fluff_uint_ftoh(FluffUInt num){
	return btol_32(num);
}

FluffULong fluff_ulong_htof(uint64_t num){
	return ltob_64(num);

}

uint64_t fluff_ulong_ftoh(FluffULong num){
	return btol_64(num);
}

#endif /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
#else /* defined(__BYTE_ORDER__) */
FluffShort fluff_short_htof(int16_t num){
	if (1 == htonl(1))
		return num;
	else
		return ltob_16(num);
}

int16_t fluff_short_ftoh(FluffShort num){
	if (1 == htonl(1))
		return num;
	else
		return btol_16(num);
}

FluffInt fluff_int_htof(int32_t num){
	if (1 == htonl(1))
		return num;
	else
		return ltob_32(num);
}

int32_t fluff_int_ftoh(FluffInt num){
	if (1 == htonl(1))
		return num;
	else
		return btol_32(num);
}

FluffLong fluff_long_htof(int64_t num){
	if (1 == htonl(1))
		return num;
	else
		return ltob_64(num);
}

int64_t fluff_long_ftoh(FluffLong num){
	if (1 == htonl(1))
		return num;
	else
		return btol_64(num);
}

FluffUShort fluff_ushort_htof(uint16_t num){
	if (1 == htonl(1))
		return num;
	else
		return ltob_16(num);
}

uint16_t fluff_ushort_ftoh(FluffUShort num){
	if (1 == htonl(1))
		return num;
	else
		return btol_16(num);
}

FluffUInt fluff_uint_htof(uint32_t num){
	if (1 == htonl(1))
		return num;
	else
		return ltob_32(num);
}

uint32_t fluff_uint_ftoh(FluffUInt num){
	if (1 == htonl(1))
		return num;
	else
		return btol_32(num);
}

FluffULong fluff_ulong_htof(uint64_t num){
	if (1 == htonl(1))
		return num;
	else
		return ltob_64(num);
}

uint64_t fluff_ulong_ftoh(FluffULong num){
	if (1 == htonl(1))
		return num;
	else
		return btol_64(num);
}

#endif /* defined(__BYTE_ORDER__) */
