#include "Json.hpp"

#include <fstream>
#include <iostream>

namespace Json
{
    std::shared_ptr<Tree> Parse(std::istream& stream)
    {
        std::string text;
        std::string line;
        while(std::getline(stream, line)) { text += line + "\n"; }
        
        Lexer lexer(text);

        std::vector<Token> tokens;
        while(true)
        {
            Token token = lexer.NextToken();
            if(token.Type == TokenType::WhiteSpace)
            {
                continue;
            }

            if(token.Type == TokenType::EndOfFile || token.Type == TokenType::Invalid)
            {
                break;
            }
            tokens.push_back(token);
        }

        Parser parser(tokens.data(), tokens.size(), std::cout);
        return std::make_shared<Tree>(parser.ParseNode());
    }
}
