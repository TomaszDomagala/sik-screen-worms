#include "unity.h"
#include "rand.h"

void setUp() {

}

void tearDown() {

}

void test_rand_1() {
	rand_set(13);
	uint32_t rand_int = rand_get();
	TEST_ASSERT_EQUAL_UINT32(13, rand_int);
}


int main() {
	UNITY_BEGIN();
	RUN_TEST(test_rand_1);

	return UNITY_END();
}
