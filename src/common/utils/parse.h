#ifndef SIK_SCREEN_WORMS_PARSE_H
#define SIK_SCREEN_WORMS_PARSE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	bool success;
	int64_t value;
} p_i64_t;

typedef struct {
	bool success;
	uint64_t value;
} p_u64_t;

p_i64_t str_to_i64(char *str);

p_u64_t str_to_u64(char *str);

#endif //SIK_SCREEN_WORMS_PARSE_H
