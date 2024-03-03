#include "run.hpp"
#include "../ast/program.hpp"
#include "../parser.hpp"
#include <cstdio>
#include <cstdlib>

void
run_usage(const char *program_name)
{
	printf("Usage: %s run\n", program_name);
}

int
run_main(int argc, const char *argv[])
{
	if (argc > 3) {
		run_usage(argv[0]);
		return EXIT_FAILURE;
	}

	std::string source_filename = argc >= 3 ? argv[2] : "main.rex";
	Parser parser = Parser(source_filename);
	Program program_ast = parser.get_program_ast();

	return EXIT_SUCCESS;
}
