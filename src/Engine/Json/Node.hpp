#pragma once

#include <concepts>
#include <string>

namespace Json
{
	class Node
	{
	public:
		virtual ~Node() = default;

		template<std::derived_from<Node> TNode>
		TNode* As() { return static_cast<TNode*>(this); }

		template<std::derived_from<Node> TNode>
		const TNode* As() const { return static_cast<const TNode*>(this); }

		[[nodiscard]] virtual Node* Clone() const = 0;
	};
}