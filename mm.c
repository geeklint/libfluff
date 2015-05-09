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

#include <stdlib.h>

static union FluffData system_type_new(size_t size){
	union FluffData as_data;

	as_data.d_size_t = size;
	return as_data;
}

static void system_type_free(union FluffData data){
	// Do nothing
}

static void * system_alloc(union FluffData as_data){
	return malloc(as_data.d_size_t);
}

const struct FluffMM mm_system = {
		&system_type_new,
		&system_type_free,
		&system_alloc,
		&malloc,
		&free,
};

const struct FluffMM * const fluff_mm_system = &mm_system;

struct CacheBlock {
    struct CacheType * type;
    struct CacheBlock * next;
};

struct CacheType {
    union FluffData type;
    size_t size;
    struct CacheBlock * cache;
    size_t count;
};

static int prev_mm_need_setup = 1;
const struct FluffMM * PREV_MM;

static union FluffData type_size;
static union FluffData block_size;

static void setup_mm(){
	if (PREV_MM == NULL){
		PREV_MM = fluff_mm_system;
	}
	type_size = PREV_MM->f_type_new(sizeof(struct CacheType));
	block_size = PREV_MM->f_type_new(sizeof(struct CacheBlock));
	prev_mm_need_setup = 0;
}

#define ENSURE_MM if (prev_mm_need_setup) setup_mm();

void fuff_mm_cache_setmm(const struct FluffMM * mm){
	if (!prev_mm_need_setup){
		PREV_MM->f_type_free(type_size);
		PREV_MM->f_type_free(block_size);
		prev_mm_need_setup = 1;
	}
	PREV_MM = mm;
	setup_mm();
}

static union FluffData cache_type_new(size_t size){
    struct CacheType * type;
    union FluffData data;

    ENSURE_MM;

    if ((type = PREV_MM->f_alloc(type_size))){
    	type->size = size;
        type->type = PREV_MM->f_type_new(size + sizeof(struct CacheBlock));
        type->cache = NULL;
        type->count = 0;
    }
    data.d_ptr = type;
    return data;
}

static void cache_clear(struct CacheType * type, size_t size){
    struct CacheBlock * cache, * tcache;
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

static void cache_type_free(union FluffData data){
	struct CacheType * type;

	type = data.d_ptr;
    cache_clear(type, 0);
    PREV_MM->f_free(type);
}

/*
static size_t cache_size(struct CacheType * type){
    return type->count * type->size;
}
*/

static void * cache_alloc(union FluffData data){
	struct CacheType * type;
    void * block;

    type = data.d_ptr;
    if (type->cache){
        block = type->cache;
        type->cache = type->cache->next;
        type->count -= 1;
    } else {
        if (!(block = PREV_MM->f_alloc(type->type))){
        	return NULL;
        }
        ((struct CacheBlock *)block)->type = type;
    }
    return block + sizeof(struct CacheBlock);
}

static void * cache_alloc_size(size_t size){
    void * block;

    size += sizeof(struct CacheBlock);
	if (!(block = PREV_MM->f_alloc_size(size))){
		return NULL;
	}
	((struct CacheBlock *)block)->type = NULL;
	return block + sizeof(struct CacheBlock);
}

static void cache_free(void * block){
    struct CacheType * type;

	block -= sizeof(struct CacheBlock);
	type = ((struct CacheBlock *)block)->type;
	if (type == NULL){
		PREV_MM->f_free(block);
	} else {
		((struct CacheBlock *)block)->next = type->cache;
		type->cache = block;
		type->count += 1;
	}
}

const struct FluffMM mm_cache = {
		&cache_type_new,
		&cache_type_free,
		&cache_alloc,
		&cache_alloc_size,
		&cache_free,
};

const struct FluffMM * const fluff_mm_cache = &mm_cache;

const struct FluffMM * fluff_mm_default = &mm_cache;
