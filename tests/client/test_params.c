#include "unity.h"

void setUp() {

}

void tearDown() {

}

void simple_test() {
	TEST_ASSERT(2 == 1 + 1);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(simple_test);
	return UNITY_END();
}
