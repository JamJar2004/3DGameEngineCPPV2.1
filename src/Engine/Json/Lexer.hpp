#pragma once

#include <Reflection.hpp>

#include <any>
#include <utility>

namespace Json
{
	
	enum class TokenType
	{
		Invalid,
		WhiteSpace,

		Comma = ',',
		Colon = ':',
		
		 OpenArray = '[',
		CloseArray = ']',

		 OpenObject = '{',
		CloseObject = '}',
	
		FloatNumber,
		IntegerNumber,
		String,
		Boolean,
		Identifier,
		EndOfFile,
	};

	struct Token
	{
		Token(std::any value, TokenType type, size_t index, size_t lineNumber) :
			Value(std::move(value)), Type(type), Index(index), LineNumber(lineNumber) {}

		const std::any  Value;
		const TokenType Type;
		const size_t    Index;
		const size_t    LineNumber;
	};

	class Lexer
	{
	public:
		explicit Lexer(std::string_view text) : m_totalString(text.data()), m_totalLength(text.length()), m_current(text.data()), m_lineNumber(1) {}

		~Lexer() = default;

		Token NextToken();
	private:
		const char* const m_totalString;
		const size_t      m_totalLength;

		const char* m_current;

		size_t m_lineNumber;
	};
}