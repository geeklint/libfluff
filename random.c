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

#include "random.h" 
/* Mersenne twiser (code adapted from wikipedia) */

#include <string.h>

#include "mm.h"

struct FluffRandom {
    int index;
    uint32_t MT[624];
};

/*
 * Memory manager
 */

static int mm_need_setup = 1;
static const struct FluffMM * MM = NULL;

static union FluffData random_size;

static void setup_mm(){
	if (MM == NULL){
		MM = fluff_mm_default;
	}
    random_size = MM->f_type_new(sizeof(struct FluffRandom));
    mm_need_setup = 0;
}

#define ENSURE_MM if (mm_need_setup) setup_mm();

void fluff_random_setmm(const struct FluffMM * mm){
	if (!mm_need_setup){
		MM->f_type_free(random_size);
		mm_need_setup = 1;
	}
	MM = mm;
	setup_mm();
}

#include <time.h>

uint32_t fluff_random_bestseed(){
	uint32_t result;
	time_t timer;
	union FluffData data;
	uint64_t uninitialized = uninitialized; // suppress warnings

	if (fluff_random_urandom(&result, sizeof(result))){
		if (time(&timer) > 0){
			if (sizeof(time_t) <= sizeof(uint32_t)){
				data.d_uint32_t = timer;
				result = (*fluff_hash_uint32_t)(data);
			} else {
				data.d_uint64_t = timer;
				result = (*fluff_hash_uint64_t)(data);
			}
		} else if (uninitialized != 0){
			data.d_uint64_t = uninitialized;
			result = (*fluff_hash_uint64_t)(data);
		} else {
			// Hopefully we don't get here so last resort:
			result = 0x83e58f34;
		}
	}
	return result;
}

// IF POSIX

#include <stdio.h>

int fluff_random_urandom(void * buf, size_t bufsize){
	FILE * urandom;
	size_t red;

	if (!(urandom = fopen("/dev/urandom", "r"))){
		return -1;
	}
	red = fread(buf, 1, bufsize, urandom);
	fclose(urandom);
	return (red == bufsize) ? 0 : 1;
}

// ENDIF /* POSIX */

struct FluffRandom * fluff_random_new(uint32_t seed){
    struct FluffRandom * self;
    int i;
    
    if ((self = MM->f_alloc(random_size))){
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

uint32_t fluff_random_int(struct FluffRandom * self){
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

#define RR_2_26 67108864.0
#define RR_2_53 9007199254740992.0

double fluff_random_random(struct FluffRandom * self){
	uint32_t a, b;

	a = fluff_random_int(self) >> 5;
	b = fluff_random_int(self) >> 6;
	return (a * RR_2_26 + b) * (1.0 / RR_2_53);
}

int fluff_random_range(struct FluffRandom * self, int min, int max){
    int diff = max - min;
    int choice;
    
    choice = (int) (fluff_random_random(self) * diff);
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
    if ((temp = MM->f_alloc_size(block))){
		while (--count){
			index = fluff_random_range(self, 0, count);
			memcpy(temp, block * count + array, block);
			memcpy(block * count + array, block * index + array, block);
			memcpy(block * index + array, temp, block);
		}
		MM->f_free(temp);
    }
}

void fluff_random_del(struct FluffRandom * self){
	MM->f_free(self);
}

