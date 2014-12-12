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

#include "mm.h"

#include <string.h>

struct FluffMMType {
    union FluffData type;
    size_t size;
    struct MMBlock * cache;
    size_t count;
};

struct MMBlock {
    struct FluffMMType * type;
    struct MMBlock * next;
};

static int DO_CACHE = 1;

static struct FluffMMDefinition * PREV_MM; // initialized below

static union FluffData TYPE_TYPE = {
		.d_size_t = sizeof(struct FluffMMType)
};

struct FluffMMType * fluff_mm_new(size_t size){
    struct FluffMMType * type;
    
    if ((type = PREV_MM->f_alloc(TYPE_TYPE))){
    	type->size = size;
        type->type = PREV_MM->f_new_type(size + sizeof(struct MMBlock));
        type->cache = NULL;
        type->count = 0;
    }
    
    return type;
}

size_t fluff_mm_cache_size(struct FluffMMType * type){
    return type->count * type->size;
}

void fluff_mm_clear(struct FluffMMType * type, size_t size){
    struct MMBlock * cache, * tcache;
    size_t count, tsize;
    
    count = type->count;
    tsize = type->size;
    tcache = type->cache;
    while (count * tsize > size && (cache = tcache)){
        tcache = cache->next;
        PREV_MM->f_free(cache);
        count -= 1;
    }
    type->count = count;
    type->size = tsize;
    type->cache = tcache;
}

void * fluff_mm_alloc(struct FluffMMType * type){
    void * block;
    
    if (type->cache){
        block = type->cache;
        type->cache = type->cache->next;
        type->count -= 1;
    } else {
        if (!(block = PREV_MM->f_alloc(type->type))){
        	return NULL;
        }
        ((struct MMBlock *)block)->type = type;
    }
    return block + sizeof(struct MMBlock);
}

void fluff_mm_free(void * block){
    struct FluffMMType * type;

	block -= sizeof(struct MMBlock);
	if (DO_CACHE){
		type = ((struct MMBlock *)block)->type;
		((struct MMBlock *)block)->next = type->cache;
		type->cache = block;
		type->count += 1;
	} else {
		PREV_MM->f_free(block);
	}
}

void fluff_mm_delete(struct FluffMMType * type){
    fluff_mm_clear(type, 0);
    PREV_MM->f_free(type);
}

void fluff_mm_do_cache(int do_cache){
	DO_CACHE = do_cache;
}

void fluff_mm_set_manager(struct FluffMMDefinition * prev_mm){
	PREV_MM = prev_mm;
	TYPE_TYPE = prev_mm->f_new_type(sizeof(struct FluffMMType));
}

static union FluffData mm_system_new_type(size_t size){
	union FluffData as_data;

	as_data.d_size_t = size;
	return as_data;
}

static void * mm_system_alloc(union FluffData as_data){
	return malloc(as_data.d_size_t);
}

struct FluffMMDefinition mm_system = {
		&mm_system_new_type,
		&mm_system_alloc,
		&free,
};

struct FluffMMDefinition * fluff_mm_definition_system = &mm_system;
static struct FluffMMDefinition * PREV_MM = &mm_system;

static union FluffData mm_fluff_new_type(size_t size){
	union FluffData as_data;

	as_data.d_ptr = fluff_mm_new(size);
	return as_data;
}

static void * mm_fluff_alloc(union FluffData as_data){
	return fluff_mm_alloc(as_data.d_ptr);
}

struct FluffMMDefinition mm_fluff = {
		&mm_fluff_new_type,
		&mm_fluff_alloc,
		&fluff_mm_free,
};

/*
 * Definition of fluff_mm manager
 */
struct FluffMMDefinition * fluff_mm_definition_fluff = &mm_fluff;
