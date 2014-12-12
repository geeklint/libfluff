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

#include <stdlib.h>

#include "data.h"

struct FluffMMType;

/*
 * Create a new managed type
 * Return the new managed type on success, NULL on failure
 */
struct FluffMMType * fluff_mm_new(size_t);

/* 
 * Get size of cache
 */
size_t fluff_mm_cache_size(struct FluffMMType *);

/*
 * Clear object cache. Size is the max size for remaining cache
 */
void fluff_mm_clear(struct FluffMMType *, size_t size);

/*
 * Allocate a new instance of the type
 * Return the new instance on success, NULL on failure
 */
void * fluff_mm_alloc(struct FluffMMType *);

/*
 * Free an instance of the type
 */
void fluff_mm_free(void *);

/*
 * Invalidate the type.
 * This function will clear the cache first
 */
void fluff_mm_delete(struct FluffMMType *);

/*
 * Enable/Disable caching
 * Disabling essentially turns fluff_mm into pass-through function calls
 */
void fluff_mm_do_cache(int do_cache);

/*
 * Definition of the new type function
 */
typedef union FluffData (*FluffMMNewTypeFunction)(size_t);

/*
 * Definition of the allocate function
 */
typedef void * (*FluffMMAllocFunction)(union FluffData);

/*
 * Definition of a memory manager
 */
struct FluffMMDefinition {
	FluffMMNewTypeFunction f_new_type;
	FluffMMAllocFunction f_alloc;
	FluffFreeFunction f_free;
};

/*
 * Set the memory manager that fluff_mm uses
 */
void fluff_mm_set_manager(struct FluffMMDefinition *);

/*
 * Definition of system malloc/free manager
 */
extern struct FluffMMDefinition * fluff_mm_definition_system;

/*
 * Definition of fluff_mm manager
 */
extern struct FluffMMDefinition * fluff_mm_definition_fluff;

#endif /* FLUFF_MM_H_ */
