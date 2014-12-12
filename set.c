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

#include "set.h"

#include "mm.h"

#define TABLE_START 16
#define ENLARGE .75
#define SHRINK .25
#define BITS_PER_BLOCK 32

/*
 * Set types
 */
struct FluffSetEnum {
	int max;
	int count;
	uint32_t * bits;
};

struct FluffSetHash {
	FluffEqualFunction equal;
	FluffHashFunction hash;
	int count;
	size_t tablesize;
	struct HashElement ** table;
};

struct HashElement {
	FluffHashValue hash;
	union FluffData data;
	struct HashElement * next;
};

struct FluffSetElement {
	struct FluffSetElementElement * head;
	int lock;
	int count;
};

struct FluffSetElementElement {
	union FluffData data;
	struct FluffSetElement * set;
	struct FluffSetElementElement * prev;
	struct FluffSetElementElement * next;
};

struct FluffSetElementIter {
	struct FluffSetElement * set;
	struct FluffSetElementElement * prev;
	struct FluffSetElementElement * next;
};


/*
 * MM
 */
static struct FluffMMGroup * mm_group = NULL;
static struct FluffMMType * setenum_size = NULL;
static struct FluffMMType * sethash_size = NULL;
static struct FluffMMType * hashel_size = NULL;
static struct FluffMMType * setelement_size = NULL;
static struct FluffMMType * element_size = NULL;
static struct FluffMMType * iter_size = NULL;

static void setup_mm_types(){
    mm_group = fluff_mm_new_g();
	setenum_size = fluff_mm_new(sizeof(struct FluffSetEnum), mm_group);
	sethash_size = fluff_mm_new(sizeof(struct FluffSetHash), mm_group);
	hashel_size = fluff_mm_new(sizeof(struct HashElement), mm_group);
	setelement_size = fluff_mm_new(sizeof(struct FluffSetElement), mm_group);
	element_size = fluff_mm_new(
			sizeof(struct FluffSetElementElement), mm_group);
	iter_size = fluff_mm_new(sizeof(struct FluffSetElementIter), mm_group);
}

#define ENSURE_MM if (!mm_group) setup_mm_types();

/*
 * Enum Set
 */
struct FluffSetEnum * fluff_set_enum_new(unsigned int max){
	struct FluffSetEnum * self;
	unsigned int n_blocks;

	ENSURE_MM;

	n_blocks = (max / BITS_PER_BLOCK) + 1;
	if ((self = fluff_mm_alloc(setenum_size))){
		if (!(self->bits = calloc(n_blocks, sizeof(uint32_t)))){
			fluff_mm_free(self);
			self = NULL;
		} else {
			self->max = max;
			self->count = 0;
		}
	}
	return self;
}

void fluff_set_enum_free(struct FluffSetEnum * self){
	free(self->bits);
	fluff_mm_free(self);
}

unsigned int fluff_set_enum_count(struct FluffSetEnum * self){
	return self->count;
}

void fluff_set_enum_add(struct FluffSetEnum * self, unsigned int n){
	unsigned int block;

	block = (n / BITS_PER_BLOCK);
	n -= block * BITS_PER_BLOCK;
	if (!(self->bits[block] & (1 << n))){
		self->count += 1;
		self->bits[block] |= (1 << n);
	}
}

int fluff_set_enum_contains(struct FluffSetEnum * self, unsigned int n){
	unsigned int block;

	block = (n / BITS_PER_BLOCK);
	n -= block * BITS_PER_BLOCK;
	return self->bits[block] & (1 << n);
}

void fluff_set_enum_remove(struct FluffSetEnum * self, unsigned int n){
	unsigned int block;

	block = (n / BITS_PER_BLOCK);
	n -= block * BITS_PER_BLOCK;
	if (self->bits[block] & (1 << n)){
		self->count -= 1;
		self->bits[block] ^= (1 << n);
	}

}

/*
 * Hash Set
 */

struct FluffSetHash * fluff_set_hash_new(
		FluffHashFunction hash, FluffEqualFunction equal){
	struct FluffSetHash * self;
	struct HashElement ** table;

	ENSURE_MM;

	if ((self = fluff_mm_alloc(sethash_size))){
		table = calloc(TABLE_START, sizeof(struct HashElement *));
		if (!table){
			fluff_mm_free(self);
			self = NULL;
		} else {
			self->table = table;
			self->tablesize = TABLE_START;
			self->count = 0;
			self->equal = equal;
			self->hash = hash;
		}
	}
	return self;
}

void fluff_set_hash_free(struct FluffSetHash * self, FluffFreeFunction freer){
	int i;
	struct HashElement * node, * to_del;

	for (i = 0; i < self->tablesize; ++i){
		while (node){
			to_del = node;
			node = node->next;
			if (freer){
				freer(to_del->data.d_ptr);
			}
			fluff_mm_free(to_del);
		}
	}
}

unsigned int fluff_set_hash_count(struct FluffSetHash * self){
	return self->count;
}

void fluff_set_hash_add(struct FluffSetHash * self, union FluffData data){
	FluffHashValue hash;
	struct HashElement ** dest;
	struct HashElement * node, * node2;
	struct HashElement ** new_table;
	size_t new_tablesize;
	int i;

	hash = self->hash(data);
	dest = self->table + (hash % self->tablesize);
	while ((node = *dest)){
		if (node->hash == hash && self->equal(node->data, data)){
			return;
		}
		dest = &(node->next);
	}
	self->count += 1;
	if (self->count * ENLARGE > self->tablesize){
		new_tablesize = self->tablesize * 2;
		new_table = calloc(new_tablesize, sizeof(struct HashElement *));
		if (new_table){
			for (i = 0; i < self->tablesize; ++i){
				node = self->table[i];
				while (node){
					node2 = node->next;
					dest = new_table + (node->hash % new_tablesize);
					node->next = *dest;
					*dest = node;
					node = node2;
				}
			}
			free(self->table);
			self->table = new_table;
			self->tablesize = new_tablesize;
			dest = new_table + (hash % new_tablesize);
			while ((node = *dest)){
				dest = &(node->next);
			}
		}
	}
	if ((node = fluff_mm_alloc(hashel_size))){
		node->data = data;
		node->hash = hash;
		node->next = NULL;
		*dest = node;
	}
}

int fluff_set_hash_contains(
		struct FluffSetHash * self, union FluffData data){
	FluffHashValue hash;
	struct HashElement * node;

	hash = self->hash(data);
	node = *(self->table + (hash % self->tablesize));
	while (node){
		if (node->hash == hash && self->equal(node->data, data)){
			return 1;
		}
		node = node->next;
	}
	return 0;
}

union FluffData fluff_set_hash_remove(
		struct FluffSetHash * self, union FluffData data){
	FluffHashValue hash;
	struct HashElement ** dest;
	struct HashElement * node;

	hash = self->hash(data);
	dest = self->table + (hash % self->tablesize);
	while ((node = *dest)){
		if (node->hash == hash && self->equal(node->data, data)){
			data = node->data;
			fluff_mm_free(node);
			*dest = NULL;
			self->count -= 1;
			return data;
		}
		dest = &(node->next);
	}
	return fluff_data_zero;
}

/*
 * Element Set
 */

struct FluffSetElement * fluff_set_element_new(){
	struct FluffSetElement * self;

	ENSURE_MM;

	if ((self = fluff_mm_alloc(setelement_size))){
		self->count = 0;
		self->head = NULL;
		self->lock = 0;
	}
	return self;
}


void fluff_set_element_free(struct FluffSetElement * self){
	if (!self->lock){
		fluff_mm_free(self);
	}
}

unsigned int fluff_set_element_count(struct FluffSetElement * self){
	return self->count;
}

void fluff_set_element_add(
		struct FluffSetElement * self,
		struct FluffSetElementElement * element){

	if (self->lock){
		return;
	}
	if (element->set == self && !(element->next || element->prev)){
		if (self->head){
			self->head->prev = element;
		}
		element->next = self->head;
		self->head = element;
	}
}


int fluff_set_element_contains(
		struct FluffSetElement * self,
		struct FluffSetElementElement * element){
	return (element->set == self && (element->next || element->prev));
}


void fluff_set_element_remove(
		struct FluffSetElement * self,
		struct FluffSetElementElement * element){

	if (self->lock){
		return;
	}
	if (element->set == self){
		if (element->next || element->prev){
			self->count -= 1;
		}
		if (element->next){
			element->next->prev = element->prev;
		}
		if (element->prev){
			element->prev->next = element->next;
		}
		element->prev = element->next = NULL;
	}
}

struct FluffSetElementElement * fluff_set_element_pop(
		struct FluffSetElement * self){
	struct FluffSetElementElement * element;

	if (self->lock){
		return NULL;
	}
	if ((element = self->head)){
		if (element->next){
			element->next->prev = NULL;
		}
		self->count -= 1;
		self->head = element->next;
		element->next = NULL;
	}
	return element;
}

struct FluffSetElementIter * fluff_set_element_iter(
		struct FluffSetElement * self){
	struct FluffSetElementIter * iter = NULL;

	if (!self->lock && (iter = fluff_mm_alloc(iter_size))){
		iter->set = self;
		iter->prev = NULL;
		iter->next = self->head;
		self->lock = 1;
	}
	return iter;
}

struct FluffSetElementElement * fluff_set_element_element_new(
		struct FluffSetElement * set){
	struct FluffSetElementElement * self;

	if ((self = fluff_mm_alloc(element_size))){
		self->set = set;
		self->next = NULL;
		self->prev = NULL;
	}
	return self;
}

void fluff_set_element_element_free(struct FluffSetElementElement * self){
	if (!(self->next || self->prev)){
		fluff_mm_free(self);
	}
}

void fluff_set_element_element_data_set(
		struct FluffSetElementElement * self, union FluffData data){
	self->data = data;
}

union FluffData fluff_set_element_element_data_get(
		struct FluffSetElementElement * self){
	return self->data;
}

void fluff_set_element_iter_free(struct FluffSetElementIter * self){
	self->set->lock = 0;
	fluff_mm_free(self);
}

struct FluffSetElementElement * fluff_set_element_iter_next(
		struct FluffSetElementIter * self){
	struct FluffSetElementElement * element;

	if ((element = self->next)){
		self->next = element->next;
		self->prev = element;
	}
	return element;
}

void fluff_set_element_iter_removelast(
		struct FluffSetElementIter * self){
	struct FluffSetElementElement * element;

	if ((element = self->prev)){
		if (self->next){
			self->next->prev = element->prev;
		}
		self->prev = element->prev;
		element->prev = element->next = NULL;
	}
}
