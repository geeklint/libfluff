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

#ifndef FLUFF_SET_H_
#define FLUFF_SET_H_

#include "data.h"
#include "mm.h"

/*
 * Enum Set
 */
struct FluffSetEnum;

/*
 * Create a new enum set
 * Returns new enum set on success, NULL on failure
 */
struct FluffSetEnum * fluff_set_enum_new(unsigned int max);

/*
 * Invalidate the enum set
 */
void fluff_set_enum_free(struct FluffSetEnum *);

/*
 * Get number of values in the set
 * Return the cardinality of the enum set
 */
unsigned int fluff_set_enum_count(struct FluffSetEnum *);

/*
 * Add a value to the enum set
 */
void fluff_set_enum_add(struct FluffSetEnum *, unsigned int n);

/*
 * Check if the enum set contains the specified value
 * Return 1 if the value is contained, 0 otherwise
 */
int fluff_set_enum_contains(struct FluffSetEnum *, unsigned int n);

/*
 * Remove a value from the enum set
 */
void fluff_set_enum_remove(struct FluffSetEnum *, unsigned int n);

/*
 * Hash set
 */
struct FluffSetHash;
struct FluffSetHashIter;

/*
 * Create a new hash set
 */
struct FluffSetHash * fluff_set_hash_new(
		FluffHashFunction, FluffEqualFunction);

/*
 * Invalidate the hash set
 */
void fluff_set_hash_free(struct FluffSetHash *, FluffFreeFunction);

/*
 * Get number of values in the set
 * Return the cardinality of the hash set
 */
unsigned int fluff_set_hash_count(struct FluffSetHash *);

/*
 * Add a value to the hash set
 */
void fluff_set_hash_add(struct FluffSetHash *, union FluffData);

/*
 * Check if the hash set contains the specified value
 * Return 1 if the value is contained, 0 otherwise
 */
int fluff_set_hash_contains(struct FluffSetHash *, union FluffData);

/*
 * Get an equivalent value from the set
 * Return 1 if the value was successfully fetched, 0 otherwise
 */
int fluff_set_hash_get(
		struct FluffSetHash *, union FluffData, union FluffData *);

/*
 * Remove a value from the hash set
 */
union FluffData fluff_set_hash_remove(struct FluffSetHash *, union FluffData);

/*
 * Get an iterator over the hash set
 * While the iterator is active, no operations may be performed on the
 * set directly.
 */
struct FluffSetHashIter * fluff_set_hash_iter(
		struct FluffSetHash *);

/*
 * Release and invalidate the hash set iterator
 */
void fluff_set_hash_iter_free(struct FluffSetHashIter *);

/*
 * Get the next element from the iterator
 * Return 1 if there was an element to get, 0 if there was not
 */
int fluff_set_hash_iter_next(
		struct FluffSetHashIter *, union FluffData *);


/*
 * Element Set
 */
struct FluffSetElement;
struct FluffSetElementElement;
struct FluffSetElementIter;

/*
 * Create a new element set
 */
struct FluffSetElement * fluff_set_element_new();

/*
 * Invalidate the element set
 */
void fluff_set_element_free(struct FluffSetElement *);

/*
 * Get number of values in the set
 * Return the cardinality of the element set
 */
unsigned int fluff_set_element_count(struct FluffSetElement *);

/*
 * Add an element to the element set
 */
void fluff_set_element_add(
		struct FluffSetElement *, struct FluffSetElementElement *);

/*
 * Check if the element set contains the specified value
 * Return 1 if the value is contained, 0 otherwise
 */
int fluff_set_element_contains(
		struct FluffSetElement *, struct FluffSetElementElement *);

/*
 * Remove an element from the hash set
 */
void fluff_set_element_remove(
		struct FluffSetElement *, struct FluffSetElementElement *);

/*
 * Remove and return an arbitrary element from the element set
 */
struct FluffSetElementElement * fluff_set_element_pop(
		struct FluffSetElement *);

/*
 * Get an iterator over the element set
 * While the iterator is active, no operations may be performed on the
 * set directly.
 */
struct FluffSetElementIter * fluff_set_element_iter(
		struct FluffSetElement *);

/*
 * Create a new element set element
 */
struct FluffSetElementElement * fluff_set_element_element_new(
		struct FluffSetElement *);

/*
 * Invalidate an element set element
 */
void fluff_set_element_element_free(struct FluffSetElementElement *);

/*
 * Set the data associated with the element set element
 */
void fluff_set_element_element_data_set(
		struct FluffSetElementElement *, union FluffData data);

/*
 * Get the data associated with the element set element
 */
union FluffData fluff_set_element_element_data_get(
		struct FluffSetElementElement *);

/*
 * Release and invalidate the element set iterator
 */
void fluff_set_element_iter_free(struct FluffSetElementIter *);

/*
 * Get the next element from the iterator
 */
struct FluffSetElementElement * fluff_set_element_iter_next(
		struct FluffSetElementIter *);

/*
 * Remove the last element returned from the iterator from the set
 */
void fluff_set_element_iter_removelast(
		struct FluffSetElementIter *);

#endif /* FLUFF_SET_H_ */
