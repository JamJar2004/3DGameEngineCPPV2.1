#pragma once

#include <Reflection.hpp>

#include <string>
#include <any>

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
		Token(const std::any& value, TokenType type, size_t index, size_t lineNumber) :
			Value(value), Type(type), Index(index), LineNumber(lineNumber) {}

		const std::any  Value;
		const TokenType Type;
		const size_t    Index;
		const size_t    LineNumber;

		//bool HasValue() const { return Value; }
	};

	class Lexer
	{
	private:
		const char* const m_totalString;
		const size_t      m_totalLength;

		const char* m_current;

		size_t m_lineNumber;
	public:
		Lexer(std::string& text) : m_totalString(text.c_str()), m_totalLength(text.length()), m_current(text.data()), m_lineNumber(1) {}

		~Lexer() {}

		Token NextToken();
	};
}