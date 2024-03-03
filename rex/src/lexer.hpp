#pragma once
#include <cstdint>
#include <fstream>
#include <string>

class Lexer {
public:
	enum class TokenType : char {
		None,
		Eof,

		/*
		 * Simple tokens
		 * (Tokens that dont hold extra data)
		 */

		// Keywords
		Let,
		Mut,
		Const,
		If,
		Else,
		Loop,
		While,
		For,
		Break,
		Continue,
		Struct,
		Enum,
		Function,
		Return,
		Type,

		// Multi-character tokens
		ExponentiationAssignment, // **=
		LeftShiftAssignment,	  // <<=
		RightShiftAssignment,	  // >>=
		Exponentiate,			  // **
		LeftShift,				  // <<
		RightShift,				  // >>
		GreaterEqual,			  // >=
		LessEqual,				  // <=
		LogicalAnd,				  // &&
		LogicalOr,				  // ||
		Equal,					  // ==
		NotEqual,				  // !=
		AdditionAssignment,		  // +=
		SubtractionAssignment,	  // -=
		MultiplicationAssignment, // *=
		DivisionAssignment,		  // /=
		ModulusAssignment,		  // %=
		BitwiseAndAssignment,	  // &=
		BitwiseOrAssignment,	  // |=
		BitwiseXorAssignment,	  // ^=

		// Single character tokens
		ExclamationMark	   = '!',
		Percent			   = '%',
		Ampersand		   = '&',
		LeftParenthesis	   = '(',
		RightParenthesis   = ')',
		Asterisk		   = '*',
		Plus			   = '+',
		Comma			   = ',',
		Minus			   = '-',
		Period			   = '.',
		Slash			   = '/',
		Colon			   = ':',
		Semicolon		   = ';',
		Less			   = '<',
		Assignment		   = '=',
		Greater			   = '>',
		LeftSquareBracket  = '[',
		RightSquareBracket = ']',
		Caret			   = '^',
		LeftCurlyBracket   = '{',
		VerticalBar		   = '|',
		RightCurlyBracket  = '}',
	};

	struct Token {
		TokenType type;
	};

	Lexer(const std::string &source_filename) noexcept;
	~Lexer() noexcept;
	Token next() noexcept;

	static bool is_whitespace(int ch) noexcept;
	static bool is_alphanumeric(int ch) noexcept;
	static bool is_alpha(int ch) noexcept;
	static bool is_numeric(int ch) noexcept;

private:
	std::ifstream source_file;

	void  consume() noexcept;
	Token next_simple() noexcept;
};
