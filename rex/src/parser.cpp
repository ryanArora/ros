#include "parser.hpp"
#include "ast/program.hpp"
#include "lexer.hpp"
#include <cstdio>

Parser::Parser(const std::string &source_filename)
	: lexer(Lexer(source_filename))
{
}

Program
Parser::get_program_ast()
{
	Lexer::Token token;
	while ((token = lexer.next()).type != Lexer::TokenType::Eof) {
		printf("token: %hhd\n", token.type);
	}

	return Program{};
}
