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

#ifndef FLUFF_CEREAL_H_
#define FLUFF_CEREAL_H_

#include <stdint.h>

#include "data.h"
#include "mm.h"
#include "stream.h"
#include "stdfmtnum.h"

enum FluffCerealTypeID {
	FluffCerealTypeIDNone = 1,
	FluffCerealTypeIDBool = 2,
	FluffCerealTypeIDInt = 3,
	FluffCerealTypeIDFloat = 4,
	FluffCerealTypeIDString = 5,
	FluffCerealTypeIDArray = 6,
	FluffCerealTypeIDObject = 7,
};

struct FluffCerealString;
struct FluffCerealArray;
struct FluffCerealObject;

typedef uint_fast8_t FluffCerealNoneType;
typedef uint_fast8_t FluffCerealBoolType;
typedef FluffInt FluffCerealIntType;
typedef FluffFloatDouble FluffCerealFloatType;

union FluffCerealType {
	FluffCerealNoneType f_none;
	FluffCerealBoolType f_bool;
	FluffCerealIntType f_int;
	FluffCerealFloatType f_float;
	struct FluffCerealString * f_string;
	struct FluffCerealArray * f_array;
	struct FluffCerealObject * f_object;
};

size_t fluff_cereal_getsize(enum FluffCerealTypeID, union FluffCerealType);

size_t fluff_cereal_dump(
		struct FluffWriteStream * stream,
		enum FluffCerealTypeID,
		union FluffCerealType);
int fluff_cereal_load(
		struct FluffReadStream * stream,
		enum FluffCerealTypeID *,
		union FluffCerealType *);

extern union FluffCerealType fluff_cereal_none;
extern union FluffCerealType fluff_cereal_true;
extern union FluffCerealType fluff_cereal_false;

struct FluffCerealString * fluff_cereal_string_new(char * src, size_t length);
void fluff_cereal_string_free(struct FluffCerealString *);
size_t fluff_cereal_string_length(struct FluffCerealString *);
void fluff_cereal_string_read(
		struct FluffCerealString *, char ** dest, size_t * length);
void fluff_cereal_string_readcopy(
		struct FluffCerealString *, char * dest, size_t maxlength);

struct FluffCerealArray * fluff_cereal_array_new(size_t length);
void fluff_cereal_array_free(struct FluffCerealArray *);
size_t fluff_cereal_array_length(struct FluffCerealArray *);
void fluff_cereal_array_set(
		struct FluffCerealArray *,
		unsigned int index,
		enum FluffCerealTypeID type,
		union FluffCerealType);
void fluff_cereal_array_get(
		struct FluffCerealArray *,
		unsigned int index,
		enum FluffCerealTypeID * type,
		union FluffCerealType * dest);

struct FluffCerealArrayBuilder;

struct FluffCerealArrayBuilder * fluff_cereal_array_builder_new();
void fluff_cereal_array_builder_free(
		struct FluffCerealArrayBuilder *);
int fluff_cereal_array_builder_append(
		struct FluffCerealArrayBuilder *,
		enum FluffCerealTypeID type,
		union FluffCerealType obj);
struct FluffCerealArray * fluff_cereal_array_builder_build(
		struct FluffCerealArrayBuilder *);

enum FluffCerealKeyID {
	FluffCerealKeyIDNone = 0,
	FluffCerealKeyIDBool = 1,
	FluffCerealKeyIDInt = 2,
	FluffCerealKeyIDFloat = 3,
	FluffCerealKeyIDString = 4,
};

union FluffCerealKey {
	FluffCerealNoneType f_none;
	FluffCerealBoolType f_bool;
	FluffCerealIntType f_int;
	FluffCerealFloatType f_float;
	struct FluffCerealString * f_string;
};

struct FluffCerealObject * fluff_cereal_object_new();
void fluff_cereal_object_free(struct FluffCerealObject *);
size_t fluff_cereal_object_count(struct FluffCerealObject *);
void fluff_cereal_object_set(
		struct FluffCerealObject *,
		enum FluffCerealKeyID,
		union FluffCerealKey,
		enum FluffCerealTypeID,
		union FluffCerealType);
int fluff_cereal_object_get(
		struct FluffCerealObject *,
		enum FluffCerealKeyID,
		union FluffCerealKey,
		enum FluffCerealTypeID *,
		union FluffCerealType *);

struct FluffCerealObjectIter;

struct FluffCerealObjectIter * fluff_cereal_object_iter(
		struct FluffCerealObject *);
void fluff_cereal_object_iter_free(struct FluffCerealObjectIter *);
int fluff_cereal_object_iter_next(
		struct FluffCerealObjectIter *,
		enum FluffCerealKeyID *,
		union FluffCerealKey *,
		enum FluffCerealTypeID *,
		union FluffCerealType *);

void fluff_cereal_setmm(const struct FluffMM *);

#endif /* FLUFF_CEREAL_H_ */
