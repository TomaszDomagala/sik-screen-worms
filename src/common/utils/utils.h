#ifndef SIK_SCREEN_WORMS_UTILS_H
#define SIK_SCREEN_WORMS_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <netdb.h>
#include <arpa/inet.h>

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

void *get_in_addr(struct sockaddr *addr);

uint32_t crc32(const void *buf, size_t size);

#endif //SIK_SCREEN_WORMS_UTILS_H
