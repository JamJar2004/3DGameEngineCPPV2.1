#pragma once

#include <unordered_map>
#include <string>

#include "Node.hpp"

namespace Json
{
	class Object final : public Node
	{
	public:
		Object() = default;

		explicit Object(const Object* other);

		~Object() override;

		      Node* Get(std::string_view name);
		const Node* Get(std::string_view name) const;

		void Add(std::string_view name, const Node* value);

		template<std::derived_from<Node> T, typename... Args>
		void Emplace(std::string& name, Args&&... args) requires std::constructible_from<T, Args...>
		{
			m_nodes[name] = new T(args...);
		}

		void Remove(std::string_view name);

		void Clear();

		auto begin() { return m_nodes.begin(); }
		auto end()   { return m_nodes.end();   }

		auto begin() const { return m_nodes.begin(); }
		auto end()   const { return m_nodes.end();   }

		Node* Clone() const override;
	private:
		std::unordered_map<std::string, Node*> m_nodes;
	};
}