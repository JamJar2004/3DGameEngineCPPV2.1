#pragma once

#include <concepts>
#include <string>

namespace Json
{
	class Node
	{
	public:
		virtual ~Node() {}

		template<std::derived_from<Node> T>
		T* As() { return (T*)this; }

		template<std::derived_from<Node> T>
		const T* As() const { return (const T*)this; }

		virtual Node* Clone() const = 0;
	};
}