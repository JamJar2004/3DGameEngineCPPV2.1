#pragma once

#include <vector>

#include "Node.hpp"

namespace Json
{
	class Array final : public Node
	{

	public:
		Array() = default;

		explicit Array(const Array* other);

		~Array() override;

		[[nodiscard]] size_t Count() const { return m_elements.size(); }

			  Node* operator[](size_t index);
		const Node* operator[](size_t index) const;

		void Add(const Node* node);
		
		template<std::derived_from<Node> T, typename... Args>
		void Emplace(Args&&... args) requires std::constructible_from<T, Args...>
		{
			m_elements.push_back(new T(args...));
		}

		void RemoveAt(size_t index);
		void Clear();

		auto begin() { return m_elements.begin(); }
		auto end()   { return m_elements.end();   }

		[[nodiscard]] auto begin() const { return m_elements.begin(); }
		[[nodiscard]] auto end()   const { return m_elements.end();   }

		[[nodiscard]] Node* Clone() const override;
	private:
		std::vector<Node*> m_elements;
	};
}