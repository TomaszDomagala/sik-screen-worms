#include "unity.h"
#include "client_args.h"

void setUp() {

}

void tearDown() {

}

void all_params() {
	char *argv[] = {
			"./screen-worms-client", "server.com",
			"-n", "myname13",
			"-p", "4321",
			"-i", "gui.com",
			"-r", "54321"
	};
	int argc = 10;

	client_args_t args;
	cp_res_t res = parse_client_args(argc, argv, &args);

	TEST_ASSERT_EQUAL(cpr_Success, res);
	TEST_ASSERT_EQUAL(4321, args.server_port);
	TEST_ASSERT_EQUAL(54321, args.gui_port);
	TEST_ASSERT_EQUAL_STRING("server.com", args.server_address);
	TEST_ASSERT_EQUAL_STRING("gui.com", args.gui_address);
	TEST_ASSERT_EQUAL_STRING("myname13", args.player_name);
}

void invalid_port() {
	char *argv[] = {
			"./screen-worms-client", "server.com",
			"-n", "myname13",
			"-p", "4321a",
	};
	int argc = 6;

	client_args_t args;
	cp_res_t res = parse_client_args(argc, argv, &args);

	TEST_ASSERT_EQUAL(cpr_InvalidArgument, res);
}

void no_server_address() {
	char *argv[] = {
			"./screen-worms-client",
			"-n", "myname13",
	};
	int argc = 3;

	client_args_t args;
	cp_res_t res = parse_client_args(argc, argv, &args);

	TEST_ASSERT_EQUAL(cpr_MissingServerAddress, res);
}

void no_player_name() {
	char *argv[] = {
			"./screen-worms-client", "server.com",
	};
	int argc = 2;

	client_args_t args;
	cp_res_t res = parse_client_args(argc, argv, &args);

	TEST_ASSERT_EQUAL(cpr_MissingPlayerName, res);
}

void missing_argument_value(){
	char *argv[] = {
			"./screen-worms-client", "server.com",
			"-p"
	};
	int argc = 3;

	client_args_t args;
	cp_res_t res = parse_client_args(argc, argv, &args);

	TEST_ASSERT_EQUAL(cpr_InvalidArgument, res);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(all_params);
	RUN_TEST(invalid_port);
	RUN_TEST(no_server_address);
	RUN_TEST(no_player_name);
	RUN_TEST(missing_argument_value);

	return UNITY_END();
}
