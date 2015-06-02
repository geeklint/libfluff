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

#ifndef FLUFF_RANDOM_H_
#define FLUFF_RANDOM_H_

#include "data.h"

/*
 * Random object
 */
struct FluffRandom;

/*
 * Get the most random seed possible.
 */
uint32_t fluff_random_bestseed();

/*
 * Fill a buffer with random bits from system random source
 * Return 0 on success, -1 otherwise
 */
int fluff_random_urandom(void * buf, size_t bufsize);

/*
 * Initialize new generator
 * Return new generator on success, NULL on failure
 */
struct FluffRandom * fluff_random_new(uint32_t seed);

/*
 * Get a random 32 bit integer
 * Return the new random number
 */
uint32_t fluff_random_int(struct FluffRandom *);

/*
 * Get a random double in the range 0 <= x < 1
 */
double fluff_random_random(struct FluffRandom *);

/*
 * Get number in range
 * Return the new bounded random number
 */
int fluff_random_range(struct FluffRandom *, int min, int max);

/*
 * Shuffle array
 * Modifies array in place
 */
void fluff_random_shuffle(
    struct FluffRandom *, void *, size_t block, size_t count);

/*
 * Invalidate generator
 */
void fluff_random_del(struct FluffRandom *);

#endif /* FLUFF_RANDOM_H_ */
