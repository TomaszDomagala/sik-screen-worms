#include "rand.h"
#include <stdint.h>

static uint32_t next_rand;

void rand_set(uint32_t rand_0) {
	next_rand = rand_0;
}

uint32_t rand_get() {
	uint32_t to_ret = next_rand;
	uint64_t mul = next_rand * 279410273;
	next_rand = (mul % 4294967291);
	return to_ret;
}