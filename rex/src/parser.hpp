#pragma once
#include "ast/program.hpp"
#include "lexer.hpp"

class Parser {
private:
	Lexer lexer;

public:
	Parser(const std::string &source_filename);
	Program get_program_ast();
};
