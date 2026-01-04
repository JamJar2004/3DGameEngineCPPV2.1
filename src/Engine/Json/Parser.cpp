#include "Parser.hpp"

#include "ValueBase.hpp"

#include <iostream>

namespace Json
{
	Parser::Parser(Token* start, size_t tokenCount, std::ostream& outputStream) :
		m_start(start), m_current(start), m_tokenCount(tokenCount), m_outputStream(outputStream) {}

	Array* Parser::ParseArray()
	{
		auto* result = new Array();
		++m_current;
		while(m_current->Type != TokenType::CloseArray)
		{
			Node* element = ParseNode();
			if(!element)
			{
				return nullptr;
			}

			result->Add(element);
			if(m_current->Type == TokenType::Comma)
			{
				++m_current;
			}
			else if(m_current->Type != TokenType::CloseArray)
			{
				Error("Expecting close array bracket: ']'");
				return nullptr;
			}
		}
		++m_current;
		return result;
	}

	Object* Parser::ParseObject()
	{
		auto* result = new Object();
		++m_current;
		while(m_current->Type != TokenType::CloseObject && m_current->Type != TokenType::EndOfFile)
		{
			if(m_current->Type != TokenType::String)
			{
				Error("Expecting string literal as property name.");
				return nullptr;
			}
			auto name = std::any_cast<std::string>(m_current->Value);
			++m_current;

			if(m_current->Type != TokenType::Colon)
			{
				Error("Expecting colon after property name: '}'");
				return nullptr;
			}

			++m_current;

			Node* element = ParseNode();
			if(!element)
			{
				return nullptr;
			}

			result->Add(name, element);
			if(m_current->Type == TokenType::Comma)
			{
				++m_current;
			}
			else if(m_current->Type != TokenType::CloseObject)
			{
				Error("Expecting close array bracket: '}'");
				return nullptr;
			}
		}
		++m_current;
		return result;
	}

	void Parser::Error(std::string_view message)
	{
		m_outputStream << message << " Line: " << m_current->LineNumber << ", Index: " << m_current->Index << std::endl;
	}

	Node* Parser::ParseNode()
	{
		Node* result = nullptr;

		if(m_current->Type == TokenType::IntegerNumber)
		{
			result = new Value(std::any_cast<int64_t>(m_current->Value));
			++m_current;
		}
		else if(m_current->Type == TokenType::FloatNumber)
		{
			result = new Value(std::any_cast<double>(m_current->Value));
			++m_current;
		}
		else if(m_current->Type == TokenType::String)
		{
			result = new Value(std::any_cast<std::string>(m_current->Value));
			++m_current;
		}
		else if(m_current->Type == TokenType::Boolean)
		{
			result = new Value(std::any_cast<bool>(m_current->Value));
			++m_current;
		}
		else if(m_current->Type == TokenType::OpenObject)
		{
			result = ParseObject();
		}
		else if(m_current->Type == TokenType::OpenArray)
		{
			result = ParseArray();
		}

		return result;
	}
}