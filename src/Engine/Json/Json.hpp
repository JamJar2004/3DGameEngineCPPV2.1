#pragma once

#include <memory>

#include "Parser.hpp"

namespace Json
{
	class Tree
	{
	public:
		Tree(Node* root) : Root(root) {}
		
		~Tree() { delete Root; }

		Node* Root;
	};

	std::shared_ptr<Tree> Parse(std::istream& stream);
}