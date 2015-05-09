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

#ifndef FLUFF_MAP_H_
#define FLUFF_MAP_H_

#include "data.h"

struct FluffMapHash;

struct FluffMapHash * fluff_map_hash_new(
		FluffHashFunction, FluffEqualFunction);

void fluff_map_hash_free(struct FluffMapHash *);

size_t fluff_map_hash_count(struct FluffMapHash *);

void fluff_map_hash_set(
		struct FluffMapHash *, union FluffData key, union FluffData value);

int fluff_map_hash_contains(
		struct FluffMapHash *, union FluffData key);

union FluffData fluff_map_hash_replace(
		struct FluffMapHash *, union FluffData key, union FluffData value);

int fluff_map_hash_get(
		struct FluffMapHash *, union FluffData key, union FluffData * value);

int fluff_map_hash_remove(
		struct FluffMapHash *, union FluffData key, union FluffData * value);

struct FluffMapHashIter;

struct FluffMapHashIter * fluff_map_hash_iter(struct FluffMapHash *);

void fluff_map_hash_iter_free(struct FluffMapHashIter *);

int fluff_map_hash_iter_next(
		struct FluffMapHashIter *,
		union FluffData * key,
		union FluffData * value);

#endif /* FLUFF_MAP_H_ */
