#include "utils.h"

#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "err.h"

p_i64_t str_to_i64(char *str) {
	p_i64_t result = {false, 0};
	char *err_ptr = NULL;
	errno = 0;

	long value = strtol(str, &err_ptr, 10);
	if (errno != 0 || err_ptr == str || *err_ptr != '\0')
		return result;

	result.success = true;
	result.value = value;
	return result;
}

p_u64_t str_to_u64(char *str) {
	p_i64_t res_i64 = str_to_i64(str);
	p_u64_t res_u64 = {res_i64.success && res_i64.value >= 0, res_i64.value};
	return res_u64;
}

uint32_t crc32(const void *buf, size_t size);

void *get_in_addr(struct sockaddr *addr) {
	switch (addr->sa_family) {
		case AF_INET:
			return &(((struct sockaddr_in *) addr)->sin_addr);
		case AF_INET6:
			return &(((struct sockaddr_in6 *) addr)->sin6_addr);
	}
	fatal("get_in_addr: invalid sa_family");
	return NULL;
}

