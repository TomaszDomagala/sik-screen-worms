#include "unity.h"
#include "server_args.h"

void setUp() {

}

void tearDown() {

}

void all_params() {
	char *argv[] = {
			"./screen-worms-server",
			"-p", "3333",
			"-s", "4444",
			"-t", "7",
			"-v", "40",
			"-w", "1000",
			"-h", "500",
	};
	int argc = 13;

	server_args_t args;
	int res = parse_server_args(argc, argv, &args);

	TEST_ASSERT_EQUAL(0, res);
	TEST_ASSERT_EQUAL(3333, args.server_port);
	TEST_ASSERT_EQUAL(4444, args.rng_seed);
	TEST_ASSERT_EQUAL(7, args.turning_speed);
	TEST_ASSERT_EQUAL(40, args.rounds_per_sec);
	TEST_ASSERT_EQUAL(1000, args.board_width);
	TEST_ASSERT_EQUAL(500, args.board_height);
}


int main() {
	UNITY_BEGIN();
	RUN_TEST(all_params);

	return UNITY_END();
}
