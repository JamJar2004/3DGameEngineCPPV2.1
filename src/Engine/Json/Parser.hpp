#pragma once

#include "Lexer.hpp"

#include "Node.hpp"
#include "Object.hpp"
#include "Array.hpp"

namespace Json
{
	class Parser
	{
	public:
		Parser(Token* start, size_t tokenCount, std::ostream& outputStream);

		Node* ParseNode();
	private:
		Token* const m_start;

		Token* m_current;

		size_t m_tokenCount;

		std::ostream& m_outputStream;

		Array*  ParseArray ();
		Object* ParseObject();

		void Error(std::string_view message);
	};
}