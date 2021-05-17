#include "parse.h"
#include <stdlib.h>
#include <errno.h>

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