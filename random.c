#include "random.h" 
/* Mersenne twiser (code adapted from wikipedia) */

#include <string.h>

#include "mm.h"

static int USE_MM = 0;

static struct FluffMMType * size = NULL;

struct FluffRandom {
    int index;
    uint32_t MT[624];
};

struct FluffRandom * fluff_random_new(uint32_t seed){
    struct FluffRandom * self;
    int i;
    
    if (USE_MM){
        if (!size){
            size = fluff_mm_new(sizeof(struct FluffRandom), NULL);
        }
        self = fluff_mm_alloc(size);
    } else {
        self = malloc(sizeof(struct FluffRandom));
    }
    
    if (self){
        self->index = 0;
        self->MT[0] = seed;
        for (i = 1; i < 624; ++i){
            self->MT[i] = i + 0x6c078965
                * (self->MT[i-1] ^ (self->MT[i-1] >> 30));
        }
    }
    return self;
}

/* Repopulate array with next set of numbers */
static void generate_numbers(struct FluffRandom * self){
    int y, i;
    
    for (i = 0; i < 624; ++i){
        y = (self->MT[i] & 0x80000000) + (self->MT[(i+1) % 624] & 0x7fffffff);
        self->MT[i] = self->MT[(i + 397) & 624] ^ (y >> 1);
        if (y & 1){ // y is odd
             self->MT[i] = self->MT[i] ^ 0x9908b0df;
        }
    }
}

uint32_t fluff_random_random(struct FluffRandom * self){
    int y;
    
    if (!self->index){
        generate_numbers(self);
    }
    y = self->MT[self->index];
    y ^= (y >> 11);
    y ^= ((y << 7) & 0x9d2c5680);
    y ^= ((y << 15) & 0xefc60000);
    y ^= (y >> 18);
    self->index = (self->index + 1) % 624;
    return y;
}

int fluff_random_range(struct FluffRandom * self, int min, int max){
    int diff = max - min;
    int choice;
    
    choice = (int) (((double) fluff_random_random(self)) / UINT32_MAX * diff);
    return choice + min;
}

static void random_shuffle_8(
        struct FluffRandom * self, uint8_t * array, size_t count){
    uint8_t temp;
    int index;

    while (--count){
        index = fluff_random_range(self, 0, count);
        temp = array[count];
        array[count] = array[index];
        array[index] = temp;
    }
}

static void random_shuffle_16(
        struct FluffRandom * self, uint16_t * array, size_t count){
    uint16_t temp;
    int index;
    
    while (--count){
        index = fluff_random_range(self, 0, count);
        temp = array[count];
        array[count] = array[index];
        array[index] = temp;
    }
}

static void random_shuffle_32(
        struct FluffRandom * self, uint32_t * array, size_t count){
    uint32_t temp;
    int index;

    while (--count){
        index = fluff_random_range(self, 0, count);
        temp = array[count];
        array[count] = array[index];
        array[index] = temp;
    }
}



void fluff_random_shuffle(
        struct FluffRandom * self, void * array, size_t block, size_t count){
	void * temp;
	int index;

    switch (block){
    	case 1:
    		random_shuffle_8(self, array, count);
    		return;
        case 2:
            random_shuffle_16(self, array, count);
            return;
        case 4:
        	random_shuffle_32(self, array, count);
        	return;
        default:
            break;
    }
    temp = malloc(block);
    while (--count){
    	index = fluff_random_range(self, 0, count);
    	memcpy(temp, block * count + array, block);
    	memcpy(block * count + array, block * index + array, block);
    	memcpy(block * index + array, temp, block);
    }
    free(temp);
}

void fluff_random_del(struct FluffRandom * self){
    if (USE_MM){
        fluff_mm_free(self);
    } else {
        free(self);
    }
}

void fluff_random_use_mm(int use){
    USE_MM = use;
}

