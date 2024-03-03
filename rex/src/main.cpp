#include "commands/run.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

void
usage(const char *program_name)
{
	printf("Usage: %s [COMMAND]\n\n", program_name);

	printf("Commands:\n");
	printf("\trun\t\tRun a program\n");
}

int
main(int argc, const char *argv[])
{
	if (argc <= 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	const char *command_str = argv[1];
	if (strcmp(command_str, "run") == 0) {
		return run_main(argc, argv);
	}

	usage(argv[0]);
	return EXIT_FAILURE;
}
