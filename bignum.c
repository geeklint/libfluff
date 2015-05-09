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

#include "bignum.h"
/*
#include <stdint.h>

#define LARGE_32 25000
#define LARGE_64 109000000000000ull

static size_t bytes_from_digits(size_t digits){
	size_t add = 0;

	if (sizeof(size_t) >= 8){
		if (digits > LARGE_64){
			add = bytes_from_digits(digits - LARGE_64);
			digits = LARGE_64;
		}
	} else {
		if (digits > LARGE_32){
			add = bytes_from_digits(digits - LARGE_32);
			digits = LARGE_32;
		}
	}

	digits *= 168783;
	digits /= 524288;
	digits += (digits * 3) + 1;
	return add + (digits / 8) + 1;
}
*/
