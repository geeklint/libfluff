#ifndef FLUFF_RANDOM_H_
#define FLUFF_RANDOM_H_

#include <stdlib.h>
#include <stdint.h>

/* Object state */
struct FluffRandom;

/* Initialize new generator */
struct FluffRandom * fluff_random_new(uint32_t seed);

/* Get random number */
uint32_t fluff_random_random(struct FluffRandom *);

/* Get number in range */
int fluff_random_range(struct FluffRandom *, int min, int max);

/* Shuffle array */
void fluff_random_shuffle(
    struct FluffRandom *, void *, size_t block, size_t count);

/* Deinitialize generator */
void fluff_random_del(struct FluffRandom *);

/* Set this module to use fluff_mm */
void fluff_random_use_mm(int use);

#endif /* FLUFF_RANDOM_H_ */
