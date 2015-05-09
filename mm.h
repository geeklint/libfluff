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

#ifndef FLUFF_MM_H_
#define FLUFF_MM_H_

#include "data.h"

/*
 * Free function prototype
 */
typedef void (*FluffFreeFunction)(void *);

/*
 * Definition of a memory manager
 */
struct FluffMM {
	union FluffData (*f_type_new)(size_t);
	void (*f_type_free)(union FluffData);
	void * (*f_alloc)(union FluffData);
	void * (*f_alloc_size)(size_t);;
	FluffFreeFunction f_free;
};

/*
 * Memory manager which will be used by modules if not overridden
 * Defaults to fluff_mm_cache
 */
extern const struct FluffMM * fluff_mm_default;

/*
 * Definition of system malloc/free manager
 */
extern const struct FluffMM * const fluff_mm_system;

/*
 * Definitions of fluff caching manager
 */
extern const struct FluffMM * const fluff_mm_cache;

/*
 * Set the memory manager used by the caching memory manager
 */
void fuff_mm_cache_setmm(const struct FluffMM *);

#endif /* FLUFF_MM_H_ */
