#ifndef FLUFF_MM_H_
#define FLUFF_MM_H_

#include <stdlib.h>

#include "data.h"

struct FluffMMType;

/*
 * Create a new managed type
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
 * Allocate and free
 */
void * fluff_mm_alloc(struct FluffMMType *);
void fluff_mm_free(void *);

/*
 * Invalidate the type.
 * This function will clear the cache first,
 *     there is no need to explicty do so
 */
void fluff_mm_delete(struct FluffMMType *);

/*
 * Enable/Disable caching
 * Disabling essentially turns fluff_mm into a pass-through function calls
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
