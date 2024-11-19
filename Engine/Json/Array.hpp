#pragma once

#include <vector>

#include "Node.hpp"

namespace Json
{
	class Array : public Node
	{
	private:
		std::vector<Node*> m_elements;
	public:
		Array() {}
		Array(const Array* other);

		virtual ~Array();

		size_t Count() const { return m_elements.size(); }

			  Node*       operator[](size_t index);
		const Node* const operator[](size_t index) const;

		void Add(Node* node);
		
		template<std::derived_from<Node> T, typename... Args>
		void Emplace(Args&&... args) requires std::constructible_from<T, Args...>
		{
			m_elements.push_back(new T(args...));
		}

		void RemoveAt(size_t index);
		void Clear();

		auto begin() { return m_elements.begin(); }
		auto end()   { return m_elements.end();   }

		auto begin() const { return m_elements.begin(); }
		auto end()   const { return m_elements.end();   }

		virtual Node* Clone() const override;
	};
}