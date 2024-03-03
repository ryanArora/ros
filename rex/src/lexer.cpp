#include "lexer.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

Lexer::Lexer(const std::string &source_filename) noexcept
	: source_file(std::ifstream(source_filename))
{
}

Lexer::~Lexer() noexcept
{
	source_file.close();
}

Lexer::Token
Lexer::next() noexcept
{
	consume();

	Token token;
	if ((token = next_simple()).type != Lexer::TokenType::None) return token;

	if (source_file.get() == EOF) {
		return Token{
			.type = Lexer::TokenType::Eof,
		};
	}

	return Token{
		.type = Lexer::TokenType::None,
	};
}

Lexer::Token
Lexer::next_simple() noexcept
{
	struct SimpleTokenMatcher {
		TokenType	token_type;
		std::string match_str;
		bool		is_word;
	};

	static const constexpr SimpleTokenMatcher simple_token_matchers[] = {
		{
			.token_type = Lexer::TokenType::Let,
			.match_str	= "let",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Mut,
			.match_str	= "mut",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Const,
			.match_str	= "const",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::If,
			.match_str	= "if",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Else,
			.match_str	= "else",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Loop,
			.match_str	= "loop",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::While,
			.match_str	= "while",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::For,
			.match_str	= "for",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Break,
			.match_str	= "break",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Continue,
			.match_str	= "continue",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Struct,
			.match_str	= "struct",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Enum,
			.match_str	= "enum",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Function,
			.match_str	= "fn",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Return,
			.match_str	= "return",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::Type,
			.match_str	= "type",
			.is_word	= true,
		},
		{
			.token_type = Lexer::TokenType::ExponentiationAssignment,
			.match_str	= "**=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::LeftShiftAssignment,
			.match_str	= "<<=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::RightShiftAssignment,
			.match_str	= ">>=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Exponentiate,
			.match_str	= "**",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::LeftShift,
			.match_str	= "<<",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::RightShift,
			.match_str	= ">>",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::GreaterEqual,
			.match_str	= ">=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::LessEqual,
			.match_str	= "<=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::LogicalAnd,
			.match_str	= "&&",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::LogicalOr,
			.match_str	= "||",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Equal,
			.match_str	= "==",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::NotEqual,
			.match_str	= "!=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::AdditionAssignment,
			.match_str	= "+=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::SubtractionAssignment,
			.match_str	= "-=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::MultiplicationAssignment,
			.match_str	= "*=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::DivisionAssignment,
			.match_str	= "/=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::ModulusAssignment,
			.match_str	= "%=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::BitwiseAndAssignment,
			.match_str	= "&=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::BitwiseOrAssignment,
			.match_str	= "|=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::BitwiseXorAssignment,
			.match_str	= "^=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::ExclamationMark,
			.match_str	= "!",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Percent,
			.match_str	= "%",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Ampersand,
			.match_str	= "&",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::LeftParenthesis,
			.match_str	= "(",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::RightParenthesis,
			.match_str	= ")",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Asterisk,
			.match_str	= "*",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Plus,
			.match_str	= "+",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Comma,
			.match_str	= ",",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Minus,
			.match_str	= "-",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Period,
			.match_str	= ".",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Slash,
			.match_str	= "/",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Colon,
			.match_str	= ":",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Semicolon,
			.match_str	= ";",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Less,
			.match_str	= "<",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Assignment,
			.match_str	= "=",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Greater,
			.match_str	= ">",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::LeftSquareBracket,
			.match_str	= "[",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::RightSquareBracket,
			.match_str	= "}",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::Caret,
			.match_str	= "^",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::LeftCurlyBracket,
			.match_str	= "{",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::VerticalBar,
			.match_str	= "|",
			.is_word	= false,
		},
		{
			.token_type = Lexer::TokenType::RightCurlyBracket,
			.match_str	= "}",
			.is_word	= false,
		},
	};

	for (SimpleTokenMatcher matcher : simple_token_matchers) {
		std::streampos pos = source_file.tellg();

		bool match = true;

		for (char match_ch : matcher.match_str) {
			int ch = source_file.get();

			if (ch != match_ch) {
				match = false;
				source_file.seekg(pos);
				break;
			}
		}

		if (matcher.is_word) {
			int ch = source_file.get();

			if (is_alphanumeric(ch) || ch == '_') {
				match = false;
			}

			source_file.seekg(-1, std::ios_base::cur);
		}

		if (match) {
			return Token{.type = matcher.token_type};
		}
	};

	return Token{
		.type = Lexer::TokenType::None,
	};
}

void
Lexer::consume() noexcept
{
	bool is_comment = false;

	while (true) {
		int ch = source_file.get();

		if (is_comment) {
			if (ch == '\n') is_comment = false;
			continue;
		}

		if (ch == '#') {
			is_comment = true;
			continue;
		}

		if (is_whitespace(ch)) {
			continue;
		}

		break;
	}

	source_file.seekg(-1, std::ios_base::cur);
}

bool
Lexer::is_whitespace(int ch) noexcept
{
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f'
		|| ch == '\r';
}

bool
Lexer::is_alphanumeric(int ch) noexcept
{
	return is_alpha(ch) || is_numeric(ch);
}

bool
Lexer::is_numeric(int ch) noexcept
{
	return '0' <= ch && ch <= '9';
}

bool
Lexer::is_alpha(int ch) noexcept
{
	return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}
