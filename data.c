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

#include "data.h"

#include <string.h>

union FluffData fluff_data_zero; // Automatically initialized to 0

static FluffHashValue hash_uint32_t_func(union FluffData data){
	// http://stackoverflow.com/a/12996028
	FluffHashValue x;

	x = data.d_uint32_t;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}

FluffHashFunction fluff_hash_uint32_t = &hash_uint32_t_func;

static FluffHashValue hash_uint64_t_func(union FluffData data){
	uint64_t x64;
	uint32_t x32;

	x64 = data.d_uint64_t;
	x32 = ((x64 >> 32) ^ x64);
	data.d_uint32_t = x32;
	return hash_uint32_t_func(data);
}

FluffHashFunction fluff_hash_uint64_t = &hash_uint64_t_func;

static FluffHashValue hash_str_func(union FluffData data){
	/* http://www.cse.yorku.ca/~oz/hash.html */
	FluffHashValue hash = 5381;
	int c;
	char * str;

	str = data.d_str;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

FluffHashFunction fluff_hash_str = &hash_str_func;

FluffHashValue fluff_hash_str_with_len(char * str, size_t length){
	/* http://www.cse.yorku.ca/~oz/hash.html */
	FluffHashValue hash = 5381;
	size_t i;

	for (i = 0; i < length; ++i){
		hash = ((hash << 5) + hash) + str[i]; /* hash * 33 + str[i] */
	}

	return hash;
}
