#ifndef FLUFF_SET_H_
#define FLUFF_SET_H_

#include "data.h"

/*
 * Every function defined here is O(1), except when noted otherwise
 */

/*
 * Enum Set
 */
struct FluffSetEnum;

/*
 * Constructor
 */
struct FluffSetEnum * fluff_set_enum_new(unsigned int max);

/*
 * Liberator
 */
void fluff_set_enum_free(struct FluffSetEnum *);

/*
 * Count method
 */
unsigned int fluff_set_enum_count(struct FluffSetEnum *);

/*
 * Add method
 */
void fluff_set_enum_add(struct FluffSetEnum *, unsigned int n);

/*
 * Contains method
 */
int fluff_set_enum_contains(struct FluffSetEnum *, unsigned int n);

/*
 * Remove method
 */
void fluff_set_enum_remove(struct FluffSetEnum *, unsigned int n);

/*
 * Hash set
 */
struct FluffSetHash;

/*
 * Constructor
 */
struct FluffSetHash * fluff_set_hash_new(
		FluffHashFunction, FluffEqualFunction);

/*
 * Liberator
 */
void fluff_set_hash_free(struct FluffSetHash *, FluffFreeFunction);

/*
 * Count method
 */
unsigned int fluff_set_hash_count(struct FluffSetHash *);

/*
 * Add method
 */
void fluff_set_hash_add(struct FluffSetHash *, union FluffData);

/*
 * Contains method
 */
int fluff_set_hash_contains(struct FluffSetHash *, union FluffData);

/*
 * Remove method
 */
union FluffData fluff_set_hash_remove(struct FluffSetHash *, union FluffData);

/*
 * Element Set
 */
struct FluffSetElement;
struct FluffSetElementElement;
struct FluffSetElementIter;

/*
 * Constructor
 */
struct FluffSetElement * fluff_set_element_new();

/*
 * Liberator
 */
void fluff_set_element_free(struct FluffSetElement *);

/*
 * Count method
 */
unsigned int fluff_set_element_count(struct FluffSetElement *);

/*
 * Add method
 */
void fluff_set_element_add(
		struct FluffSetElement *, struct FluffSetElementElement *);

/*
 * Contains method
 */
int fluff_set_element_contains(
		struct FluffSetElement *, struct FluffSetElementElement *);

/*
 * Remove method
 */
void fluff_set_element_remove(
		struct FluffSetElement *, struct FluffSetElementElement *);

/*
 * Pop method
 */
struct FluffSetElementElement * fluff_set_element_pop(
		struct FluffSetElement *);

/*
 * Iter method
 */
struct FluffSetElementIter * fluff_set_element_iter(
		struct FluffSetElement *);

/*
 * Element Set Element
 */

/*
 * Constructor
 */
struct FluffSetElementElement * fluff_set_element_element_new(
		struct FluffSetElement *);

/*
 * Liberator
 */
void fluff_set_element_element_free(struct FluffSetElementElement *);

/*
 * Data setter
 */
void fluff_set_element_element_data_set(
		struct FluffSetElementElement *, union FluffData data);

/*
 * Data getter
 */
union FluffData fluff_set_element_element_data_get(
		struct FluffSetElementElement *);

/*
 * Element Set Iter
 */

/*
 * Liberator
 */
void fluff_set_element_iter_free(struct FluffSetElementIter *);

/*
 * Next method
 */
struct FluffSetElementElement * fluff_set_element_iter_next(
		struct FluffSetElementIter *);

/*
 * Removelast method
 */
void fluff_set_element_iter_removelast(
		struct FluffSetElementIter *);


#endif /* FLUFF_SET_H_ */
