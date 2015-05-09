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

#include <limits.h>
#include <string.h>

#define TABLE_START 16
#define ENLARGE .75
#define SHRINK .25

/*
 * Set types
 */
struct FluffSetEnum {
	int max;
	int count;
	char * bits;
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
 * Memory manager
 */

static int mm_need_setup = 1;
static const struct FluffMM * MM = NULL;

static union FluffData setenum_size;
static union FluffData sethash_size;
static union FluffData hashel_size;
static union FluffData setelement_size;
static union FluffData element_size;
static union FluffData iter_size;

static void setup_mm(){
	if (MM == NULL){
		MM = fluff_mm_default;
	}
    setenum_size = MM->f_type_new(sizeof(struct FluffSetEnum));
    sethash_size = MM->f_type_new(sizeof(struct FluffSetHash));
    hashel_size = MM->f_type_new(sizeof(struct HashElement));
    setelement_size = MM->f_type_new(sizeof(struct FluffSetElement));
    element_size = MM->f_type_new(sizeof(struct FluffSetElementElement));
    iter_size = MM->f_type_new(sizeof(struct FluffSetElementIter));

}

#define ENSURE_MM if (mm_need_setup) setup_mm();

void fluff_set_setmm(const struct FluffMM * mm){
	if (!mm_need_setup){
		MM->f_type_free(setenum_size);
		MM->f_type_free(sethash_size);
		MM->f_type_free(hashel_size);
		MM->f_type_free(setelement_size);
		MM->f_type_free(element_size);
		MM->f_type_free(iter_size);
		mm_need_setup = 1;
	}
	MM = mm;
	setup_mm();
}

/*
 * Enum Set
 */
struct FluffSetEnum * fluff_set_enum_new(unsigned int max){
	struct FluffSetEnum * self;
	unsigned int n_bytes;

	ENSURE_MM;

	n_bytes = (max + CHAR_BIT - 1) / CHAR_BIT;
	if ((self = MM->f_alloc(setenum_size))){
		if (!(self->bits = MM->f_alloc_size(n_bytes))){
			MM->f_free(self);
			self = NULL;
		} else {
			memset(self->bits, 0, n_bytes);
			self->max = max;
			self->count = 0;
		}
	}
	return self;
}

void fluff_set_enum_free(struct FluffSetEnum * self){
	MM->f_free(self->bits);
	MM->f_free(self);
}

unsigned int fluff_set_enum_count(struct FluffSetEnum * self){
	return self->count;
}

static inline int set_enum_contains_base(
		struct FluffSetEnum * self, unsigned int block, unsigned int n){
	return self->bits[block] & (1 << n);
}

void fluff_set_enum_add(struct FluffSetEnum * self, unsigned int n){
	unsigned int block;

	block = (n / CHAR_BIT);
	n %= CHAR_BIT;
	if (!set_enum_contains_base(self, block, n)){
		self->count += 1;
		self->bits[block] |= (1 << n);
	}
}

int fluff_set_enum_contains(struct FluffSetEnum * self, unsigned int n){
	unsigned int block;

	block = (n / CHAR_BIT);
	n %= CHAR_BIT;
	return set_enum_contains_base(self, block, n);
}

void fluff_set_enum_remove(struct FluffSetEnum * self, unsigned int n){
	unsigned int block;

	block = (n / CHAR_BIT);
	n %= CHAR_BIT;
	if (set_enum_contains_base(self, block, n)){
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

	if ((self = MM->f_alloc(sethash_size))){
		table = MM->f_alloc_size(TABLE_START * sizeof(struct HashElement *));
		if (!table){
			MM->f_free(self);
			self = NULL;
		} else {
			memset(table, 0, TABLE_START * sizeof(struct HashElement *));
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
			MM->f_free(to_del);
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
		new_table = MM->f_alloc_size(
				new_tablesize * sizeof(struct HashElement *));
		if (new_table){
			memset(new_table, 0, new_tablesize * sizeof(struct HashElement *));
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
			MM->f_free(self->table);
			self->table = new_table;
			self->tablesize = new_tablesize;
			dest = new_table + (hash % new_tablesize);
			while ((node = *dest)){
				dest = &(node->next);
			}
		}
	}
	if ((node = MM->f_alloc(hashel_size))){
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

int fluff_set_hash_get(
		struct FluffSetHash * self,
		union FluffData data,
		union FluffData * dest){
	FluffHashValue hash;
	struct HashElement * node;

	hash = self->hash(data);
	node = *(self->table + (hash % self->tablesize));
	while (node){
		if (node->hash == hash && self->equal(node->data, data)){
			if (dest){
				*dest = node->data;
			}
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
			MM->f_free(node);
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

	if ((self = MM->f_alloc(setelement_size))){
		self->count = 0;
		self->head = NULL;
		self->lock = 0;
	}
	return self;
}


void fluff_set_element_free(struct FluffSetElement * self){
	if (!self->lock){
		MM->f_free(self);
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

	if (!self->lock && (iter = MM->f_alloc(iter_size))){
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

	if ((self = MM->f_alloc(element_size))){
		self->set = set;
		self->next = NULL;
		self->prev = NULL;
	}
	return self;
}

void fluff_set_element_element_free(struct FluffSetElementElement * self){
	if (!(self->next || self->prev)){
		MM->f_free(self);
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
	MM->f_free(self);
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
