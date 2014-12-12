#include "data.h"

static FluffHashValue fluff_hash_hash_func(union FluffData data){
	// http://stackoverflow.com/a/12996028
	FluffHashValue x;

	x = data.d_hash;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}

FluffHashFunction fluff_hash_hash = &fluff_hash_hash_func;

union FluffData fluff_data_zero;
