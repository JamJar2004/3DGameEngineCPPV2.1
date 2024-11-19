#include "Lexer.hpp"

#include <string>
#include <charconv>
#include <vector>
#include <sstream>

namespace Json
{
    Token Lexer::NextToken()
    {
        std::any value = nullptr;

        TokenType type = TokenType::Invalid;

        const char* const start = m_current;
        if(*start == '\0')
        {
            ++m_current;
            type = TokenType::EndOfFile;
        }
        else if(*start == '{')
        {
            ++m_current;
            type = TokenType::OpenObject;
        }
        else if(*start == '}')
        {
            ++m_current;
            type = TokenType::CloseObject;
        }
        else if(*start == '[')
        {
            ++m_current;
            type = TokenType::OpenArray;
        }
        else if(*start == ']')
        {
            ++m_current;
            type = TokenType::CloseArray;
        }
        else if(*start == ':')
        {
            ++m_current;
            type = TokenType::Colon;
        }
        else if(*start == ',')
        {
            ++m_current;
            type = TokenType::Comma;
        }
        else if(*start == '\r')
        {
            value = nullptr;

            ++m_lineNumber;
            ++m_current;
            if(*m_current == '\n')
            {
                ++m_current;
            }

            type = TokenType::WhiteSpace;
        }
        else if(*start == '\n')
        {
            value = nullptr;
            ++m_lineNumber;
            ++m_current;

            type = TokenType::WhiteSpace;
        }
        else if(*start == ' ')
        {
            value = nullptr;
            ++m_current;

            type = TokenType::WhiteSpace;
        }
        else if(std::isdigit(*start) || *start == '-')
        {
            bool isNegative = *start == '-';
            bool foundDecimalPoint = false;
            do
            {
                ++m_current;

                if(!foundDecimalPoint && *m_current == '.')
                {
                    foundDecimalPoint = true;
                    ++m_current;
                }
            }
            while(std::isdigit(*m_current));

            if(foundDecimalPoint)
            {
                double floatValue = 0.0;
                std::from_chars(start, m_current, floatValue);
                value = floatValue;
                type = TokenType::FloatNumber;
            }
            else
            {
                int64_t intValue = 0;
                std::from_chars(start, m_current, intValue);
                value = intValue;
                type = TokenType::IntegerNumber;
            }
        }
        else if(*start == '"')
        {
            std::stringstream characters;
            ++m_current;
            while(true)
            {
                characters << *m_current;

                if(*m_current == '\\')
                {
                    ++m_current;
                    switch(*m_current)
                    {
                        case 'n': characters << '\n'; break;
                        case 'r': characters << '\r'; break;
                        case 't': characters << '\t'; break;
                        case '"': characters << '"'; break;
                        case '\'': characters << '\''; break;
                        case '\\': characters << '\\'; break;
                    }
                }

                ++m_current;

                if(*m_current == '"')
                {
                    break;
                }
            } 
            
            ++m_current;

            value = characters.str();
            type = TokenType::String;
        }
        else if(std::isalpha(*start))
        {
            do
            {
                ++m_current;
            } 
            while(std::isalpha(*m_current));

            std::string identifier = std::string(start, m_current);

            if(identifier == "true")
            {
                value = true;
                type = TokenType::Boolean;
            }
            else if(identifier == "false")
            {
                value = false;
                type = TokenType::Boolean;
            }
            else
            {
                value = identifier;
                type = TokenType::Identifier;
            }
        }

        return Token(value, type, start - m_totalString, m_lineNumber);
    }
}
