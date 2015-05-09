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
#include "cereal.h"

#include <string.h>

#include "set.h"

struct FluffCerealString {
	uint64_t global_index;
	uint64_t global_id;
	size_t length;
	char * string;
};

struct FluffCerealArray {
	uint64_t global_index;
	uint64_t global_id;
	size_t length;
	enum FluffCerealTypeID * arr_types;
	union FluffCerealType * arr_values;
};

struct BuilderNode {
	enum FluffCerealTypeID type;
	union FluffCerealType value;
	struct BuilderNode * prev;
};

struct FluffCerealArrayBuilder {
	size_t count;
	struct BuilderNode * node;
};

struct FluffCerealObject {
	uint64_t global_index;
	uint64_t global_id;
	struct FluffSetHash * dict;
};

struct ObjectEntry {
	enum FluffCerealKeyID key_type;
	union FluffCerealKey key_value;
	enum FluffCerealTypeID value_type;
	union FluffCerealType value_value;
};

union FluffCerealType fluff_cereal_none = {
		.f_none = 0,
};

union FluffCerealType fluff_cereal_true = {
	.f_bool = 1,
};

union FluffCerealType fluff_cereal_false = {
	.f_bool = 0,
};


/*
 * Memory manager
 */

static int mm_need_setup = 1;
static const struct FluffMM * MM = NULL;

static union FluffData string_size;
static union FluffData array_size;
static union FluffData arraybuilder_size;
static union FluffData buildernode_size;
static union FluffData object_size;
static union FluffData entry_size;

static void setup_mm(){
	if (MM == NULL){
		MM = fluff_mm_system;
	}
	string_size = MM->f_type_new(sizeof(struct FluffCerealString));
	array_size = MM->f_type_new(sizeof(struct FluffCerealArray));
	arraybuilder_size = MM->f_type_new(
			sizeof(struct FluffCerealArrayBuilder));
	buildernode_size = MM->f_type_new(sizeof(struct BuilderNode));
	object_size = MM->f_type_new(sizeof(struct FluffCerealObject));
	entry_size = MM->f_type_new(sizeof(struct ObjectEntry));
}

#define ENSURE_MM if (mm_need_setup) setup_mm();

static uint64_t GLOBAL_RECURSE_ID = 0;

/*
 * Getsize function
 */

static size_t GETSIZE_GLOBAL_SIZE;

#define GETSIZE_SIZEOF_ELEMENT (sizeof(uint8_t) + sizeof(uint64_t))

static void getsize_recursive_key(
		enum FluffCerealKeyID type, union FluffCerealKey value){
	switch (type){
		case FluffCerealKeyIDString:
			if (value.f_string->global_id != GLOBAL_RECURSE_ID){
				GETSIZE_GLOBAL_SIZE += 8;
				GETSIZE_GLOBAL_SIZE += value.f_string->length;
				value.f_string->global_id = GLOBAL_RECURSE_ID;
			}
			break;
		default:
			break;
	}
}

static void getsize_recursive(
		enum FluffCerealTypeID type, union FluffCerealType value){
	uint64_t i, length;
	enum FluffCerealTypeID * arr_types;
	union FluffCerealType * arr_values;
	struct FluffCerealObjectIter * iter;
	enum FluffCerealKeyID key_type;
	union FluffCerealKey key_value;
	enum FluffCerealTypeID value_type;
	union FluffCerealType value_value;

	switch (type) {
		case FluffCerealTypeIDString:
			if (value.f_string->global_id != GLOBAL_RECURSE_ID){
				GETSIZE_GLOBAL_SIZE += 8;
				GETSIZE_GLOBAL_SIZE += value.f_string->length;
				value.f_string->global_id = GLOBAL_RECURSE_ID;
			}
			break;
		case FluffCerealTypeIDArray:
			if (value.f_array->global_id != GLOBAL_RECURSE_ID){
				length = value.f_array->length;
				arr_types = value.f_array->arr_types;
				arr_values = value.f_array->arr_values;
				GETSIZE_GLOBAL_SIZE += 8;
				GETSIZE_GLOBAL_SIZE += 9 * length;
				value.f_array->global_id = GLOBAL_RECURSE_ID;
				for (i = 0; i < length; ++i){
					getsize_recursive(arr_types[i], arr_values[i]);
				}
			}
			break;
		case FluffCerealTypeIDObject:
			if (value.f_object->global_id != GLOBAL_RECURSE_ID){
				GETSIZE_GLOBAL_SIZE += 8;
				GETSIZE_GLOBAL_SIZE += 18 * fluff_set_hash_count(
						value.f_object->dict);
				iter = fluff_cereal_object_iter(value.f_object);
				while (fluff_cereal_object_iter_next(
						iter,
						&key_type, &key_value,
						&value_type, &value_value)){
					getsize_recursive_key(key_type, key_value);
					getsize_recursive(value_type, value_value);
				}
				fluff_cereal_object_iter_free(iter);
			}
			break;
		default:
			break;
	}
}

size_t fluff_cereal_getsize(
		enum FluffCerealTypeID type, union FluffCerealType value){
	GLOBAL_RECURSE_ID += 1;
	GETSIZE_GLOBAL_SIZE = 34;
	getsize_recursive(type, value);
	return GETSIZE_GLOBAL_SIZE;
}

/*
 * Dump function
 */

size_t fluff_cereal_dump(
		struct FluffWriteStream * stream,
		enum FluffCerealTypeID,
		union FluffCerealType){
	GLOBAL_RECURSE_ID += 1;
}

/*
 * Load function
 */

int fluff_cereal_load(
		struct FluffReadStream * stream,
		enum FluffCerealTypeID *,
		union FluffCerealType *){

}

/*
 * String methods
 */

struct FluffCerealString * fluff_cereal_string_new(char * src, size_t length){
	struct FluffCerealString * self;

	ENSURE_MM;

	if ((self = MM->f_alloc(string_size))){
		if ((self->string = MM->f_alloc_size(length))){
			self->global_id = 0;
			self->length = length;
			memcpy(self->string, src, length);
		} else {
			MM->f_free(self);
			self = NULL;
		}
	}
	return self;
}

void fluff_cereal_string_free(struct FluffCerealString * self){
	MM->f_free(self->string);
	MM->f_free(self);
}

size_t fluff_cereal_string_length(struct FluffCerealString * self){
	return self->length;
}

void fluff_cereal_string_read(
		struct FluffCerealString * self, char ** dest, size_t * length){
	*dest = self->string;
	*length = self->length;
}

void fluff_cereal_string_readcopy(
		struct FluffCerealString * self, char * dest, size_t maxlength){
	memcpy(dest, self->string, maxlength);
}

/*
 * Array Methods
 */

struct FluffCerealArray * fluff_cereal_array_new(size_t length){
	struct FluffCerealArray * self;

	ENSURE_MM;

	if ((self = MM->f_alloc(array_size))){
		if ((self->arr_types = MM->f_alloc_size(
				length * sizeof(enum FluffCerealTypeID)))){
			if ((self->arr_values = MM->f_alloc_size(
					length * sizeof(union FluffCerealType)))){
				self->global_id = 0;
				self->length = length;
			} else {
				MM->f_free(self->arr_types);
				MM->f_free(self);
				self = NULL;
			}
		} else {
			MM->f_free(self);
			self = NULL;

		}
	}
	return self;
}

void fluff_cereal_array_free(struct FluffCerealArray * self){
	MM->f_free(self->arr_values);
	MM->f_free(self->arr_types);
	MM->f_free(self);
}

size_t fluff_cereal_array_length(struct FluffCerealArray * self){
	return self->length;
}

void fluff_cereal_array_set(
		struct FluffCerealArray * self,
		unsigned int index,
		enum FluffCerealTypeID type,
		union FluffCerealType value){
	self->arr_types[index] = type;
	self->arr_values[index] = value;
}

void fluff_cereal_array_get(
		struct FluffCerealArray * self,
		unsigned int index,
		enum FluffCerealTypeID * type,
		union FluffCerealType * dest){
	*type = self->arr_types[index];
	*dest = self->arr_values[index];
}

/*
 * ArrayBuilder methods
 */

struct FluffCerealArrayBuilder * fluff_cereal_array_builder_new(){
	struct FluffCerealArrayBuilder * self;

	if ((self = MM->f_alloc(arraybuilder_size))){
		self->count = 0;
		self->node = NULL;
	}
	return self;
}
void fluff_cereal_array_builder_free(
		struct FluffCerealArrayBuilder * self){
	struct BuilderNode * node, * to_del;

	node = self->node;
	while (node){
		to_del = node;
		node = node->prev;
		MM->f_free(to_del);
	}
	MM->f_free(self);
}

int fluff_cereal_array_builder_append(
		struct FluffCerealArrayBuilder * self,
		enum FluffCerealTypeID type,
		union FluffCerealType value){
	struct BuilderNode * node;

	if ((node = MM->f_alloc(buildernode_size))){
		node->type = type;
		node->value = value;
		node->prev = self->node;
		self->node = node;
		self->count += 1;
		return 0;
	}
	return -1;
}

struct FluffCerealArray * fluff_cereal_array_builder_build(
		struct FluffCerealArrayBuilder * self){
	size_t count;
	struct FluffCerealArray * array;
	struct BuilderNode * node;

	count = self->count;
	if ((array = fluff_cereal_array_new(count))){
		node = self->node;
		while (count--){
			fluff_cereal_array_set(array, count, node->type, node->value);
			node = node->prev;
		}
	}
	return array;
}

/*
 * Object methods
 */

static FluffHashValue object_hash_func(union FluffData data){
	struct ObjectEntry * entry;

	entry = data.d_ptr;
	switch (entry->key_type){
		case FluffCerealKeyIDNone:
			return 0; // Probably a bad idea
		case FluffCerealKeyIDBool:
			if (entry->key_value.f_bool){
				data.d_uint64_t = 1;
			} else {
				data.d_uint64_t = 0;
			}
			return fluff_hash_uint64_t(data);
		case FluffCerealKeyIDInt:
			data.d_uint64_t = entry->key_value.f_int;
			return fluff_hash_uint64_t(data);
		case FluffCerealKeyIDFloat:
			data.d_uint64_t = entry->key_value.f_float;
			return fluff_hash_uint64_t(data);
		case FluffCerealKeyIDString:
			return fluff_hash_str_with_len(
					entry->key_value.f_string->string,
					entry->key_value.f_string->length);
		default:
			return 0;
	}
}

static int object_equals_function(union FluffData a, union FluffData b){
	struct ObjectEntry * entry_a, * entry_b;
	struct FluffCerealString * str_a, * str_b;

	entry_a = a.d_ptr;
	entry_b = b.d_ptr;
	if (entry_a->key_type == entry_b->key_type){
		switch (entry_a->key_type){
			case FluffCerealKeyIDNone:
				return 1;
			case FluffCerealKeyIDBool:
				return entry_a->key_value.f_bool && entry_b->key_value.f_bool;
			case FluffCerealKeyIDInt:
				return (entry_a->key_value.f_int == entry_b->key_value.f_int);
			case FluffCerealKeyIDFloat:
				return (entry_a->key_value.f_float
						== entry_b->key_value.f_float);
			case FluffCerealKeyIDString:
				str_a = entry_a->key_value.f_string;
				str_b = entry_b->key_value.f_string;
				return (str_a->length == str_b->length &&
						!memcmp(str_a->string, str_b->string, str_a->length));
			default:
				break;
		}
	}
	return 0;
}

struct FluffCerealObject * fluff_cereal_object_new(){
	struct FluffCerealObject * self;

	if ((self = MM->f_alloc(object_size))){
		if ((self->dict = fluff_set_hash_new(
				&object_hash_func, &object_equals_function))){
			self->global_id = 0;
		} else {
			MM->f_free(self);
			self = NULL;
		}
	}
	return self;
}

void fluff_cereal_object_free(struct FluffCerealObject * self){
	fluff_set_hash_free(self->dict, NULL);
	MM->f_free(self);
}

size_t fluff_cereal_object_count(struct FluffCerealObject * self){
	return fluff_set_hash_count(self->dict);
}

void fluff_cereal_object_set(
		struct FluffCerealObject * self,
		enum FluffCerealKeyID key_type,
		union FluffCerealKey key_value,
		enum FluffCerealTypeID value_type,
		union FluffCerealType value_value){
	struct ObjectEntry * entry, lookup;
	union FluffData data;

	lookup.key_type = key_type;
	lookup.key_value = key_value;
	data.d_ptr = &lookup;
	if (fluff_set_hash_get(self->dict, data, &data)){
		entry = data.d_ptr;
		entry->value_type = value_type;
		entry->value_value = value_value;
	} else {
		if ((entry = MM->f_alloc(entry_size))){
			entry->key_type = key_type;
			entry->key_value = key_value;
			entry->value_type = value_type;
			entry->value_value = value_value;
			data.d_ptr = entry;
			fluff_set_hash_add(self->dict, data);
		} else {
			// TODO return error
		}
	}
}

int fluff_cereal_object_get(
		struct FluffCerealObject * self,
		enum FluffCerealKeyID key_type,
		union FluffCerealKey key_value,
		enum FluffCerealTypeID * value_type,
		union FluffCerealType * value_value){
	struct ObjectEntry * entry, lookup;
	union FluffData data;

	lookup.key_type = key_type;
	lookup.key_value = key_value;
	data.d_ptr = &lookup;
	if (fluff_set_hash_get(self->dict, data, &data)){
		entry = data.d_ptr;
		*value_type = entry->value_type;
		*value_value = entry->value_value;
		return 1;
	} else {
		return 0;
	}
}

struct FluffCerealObjectIter * fluff_cereal_object_iter(
		struct FluffCerealObject * self){
	return (struct FluffCerealObjectIter *)fluff_set_hash_iter(self->dict);
}

void fluff_cereal_object_iter_free(struct FluffCerealObjectIter * self){
	fluff_set_hash_iter_free((struct FluffSetHashIter *)self);
}

int fluff_cereal_object_iter_next(
		struct FluffCerealObjectIter * self,
		enum FluffCerealKeyID * key_type,
		union FluffCerealKey * key_value,
		enum FluffCerealTypeID * value_type,
		union FluffCerealType * value_value){
	union FluffData data;
	struct ObjectEntry * entry;

	if (fluff_set_hash_iter_next((struct FluffSetHashIter *)self, &data)){
		entry = data.d_ptr;
		*key_type = entry->key_type;
		*key_value = entry->key_value;
		*value_type = entry->value_type;
		*value_value = entry->value_value;
		return 1;
	}
	return 0;
}

void fluff_cereal_setmm(const struct FluffMM * mm){
	MM = mm;
}

/*
Format specification:
  - 1B magic number
  - 1B Type of element
  - VL Element content
    ...
  - 1B Type of root element
  - VL root element content
  - 1B Null byte

Strings:
  - 8B Number of characters
  - VL Characters

Arrays:
  - 8B Number of elements
  - 1B Type of element 0
  - 8B Value of element 0
    ...
  - 1B Type of element N
  - 8B Value of element N

Objects:
  - 8B Number of entries
  - 1B Type of entry 0 key
  - 8B Value of entry 0 key
  - 1B Type of entry 0 value
  - 8B Value of entry 0 value
    ...
  - 1B Type of entry N key
  - 8B Value of entry N key
  - 1B Type of entry N value
  - 8B Value of entry N value
*/
