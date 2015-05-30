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

#ifndef FLUFF_DATA_H_
#define FLUFF_DATA_H_

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

/*
 * Value used for hash functions.
 * Is an unsigned integer type guaranteed to be exactly 32 bits
 */
typedef uint32_t FluffHashValue;

/*
 * Union of common built in types
 */
union FluffData {
	void * d_ptr;
	char d_char;
	signed char d_s_char;
	unsigned char d_u_char;
	signed short d_s_short;
	unsigned short d_u_short;
	signed int d_s_int;
	unsigned int d_u_int;
	signed long d_s_long;
	unsigned long d_u_long;
	signed long long d_s_long_long;
	unsigned long long d_u_long_long;
	float d_float;
	double d_double;
	size_t d_size_t;
	char * d_str;
	int8_t d_int8_t;
	uint8_t d_uint8_t;
	int16_t d_int16_t;
	uint16_t d_uint16_t;
	int32_t d_int32_t;
	uint32_t d_uint32_t;
	int64_t d_int64_t;
	uint64_t d_uint64_t;
	FluffHashValue d_hash;
};

/*
 * Predfined data value with all fields set to 0
 */
extern union FluffData fluff_data_zero;

/*
 * Equal function prototype
 * Return 1 if the two parameters are equal, 0 if not
 */
typedef int (*FluffEqualFunction)(union FluffData, union FluffData);

/*
 * Hash function prototype
 */
typedef FluffHashValue (*FluffHashFunction)(union FluffData);

/*
 * Predefined hash function which hashes the value stored in d_uint32_t
 */
extern FluffHashFunction fluff_hash_uint32_t;

/*
 * Predefined hash function which hashes the value stored in d_uint64_t
 */
extern FluffHashFunction fluff_hash_uint64_t;

/*
 * Predefined hash function which hashes the value stored in d_str
 */
extern FluffHashFunction fluff_hash_str;

/*
 * Hash function to hash a string with a length (e.g. to include null)
 */
FluffHashValue fluff_hash_str_with_len(char * str, size_t length);

#endif /* FLUFF_DATA_H_ */
