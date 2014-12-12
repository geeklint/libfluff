#ifndef FLUFF_DATA_H_
#define FLUFF_DATA_H_

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint32_t FluffHashValue;

/*
 * Union of common built in types
 */
union FluffData {
	void * d_ptr;
	_Bool d_bool;
	char d_char;
	signed char d_s_char;
	unsigned char d_u_char;
	signed short d_s_short;
	unsigned short d_u_short;
	signed int d_s_int;
	unsigned int d_u_int;
	signed long d_s_long;
	unsigned long d_u_long;
	signed long long d_s_long_long;
	unsigned long long d_u_long_long;
	float d_float;
	double d_double;
	long double d_long_double;
	size_t d_size_t;
	ptrdiff_t d_ptrdiff_t;
	FluffHashValue d_hash;
};

/*
 * Free function prototype
 */
typedef void (*FluffFreeFunction)(void *);

/*
 * Equal function prototype
 */
typedef int (*FluffEqualFunction)(union FluffData, union FluffData);

/*
 * Hash function prototype
 */
typedef FluffHashValue (*FluffHashFunction)(union FluffData);

/*
 * Predefined hash function which hashes the value stored in d_hash
 */
extern FluffHashFunction fluff_hash_hash;

/*
 * Predfined data value with all fields set to 0
 */
extern union FluffData fluff_data_zero;

#endif /* FLUFF_DATA_H_ */
