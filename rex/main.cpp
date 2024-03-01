#include "main.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* main */

void usage(const char* program_name) {
	printf("Usage: %s [COMMAND]\n\n", program_name);

	printf("Commands:\n");
	printf("\trun\t\tRun the project\n");
}

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	
	const char* command_str = argv[1];
	if (strcmp(command_str, "run") == 0) {
		return run_main(argc, argv);
	}

	usage(argv[0]);
	return EXIT_FAILURE;
}

/* run */

void run_usage(const char* program_name) {
	printf("Usage: %s run\n", program_name);
}

int run_main(int argc, char* argv[]) {
	if (argc > 2) {
		run_usage(argv[0]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}