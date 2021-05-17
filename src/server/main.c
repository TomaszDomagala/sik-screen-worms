#include <stdio.h>
#include "server_args.h"
#include "err.h"

int main(int argc, char *argv[]) {

	server_args_t args;
	int res = parse_server_args(argc, argv, &args);
	if (res != 0) {
		fatal("Usage: [-p n] [-s n] [-t n] [-v n] [-w n] [-h n]");
	}

	return 0;
}
