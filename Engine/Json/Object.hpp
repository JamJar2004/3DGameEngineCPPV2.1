#pragma once

#include <unordered_map>
#include <string>

#include "Node.hpp"

namespace Json
{
	class Object : public Node
	{
	private:
		std::unordered_map<std::string, Node*> m_nodes;
	public:
		Object() {}
		Object(const Object* other);

		virtual ~Object();

		      Node* Get(const std::string& name);
		const Node* Get(const std::string& name) const;

		void Add(const std::string& name, Node* value);

		template<std::derived_from<Node> T, typename... Args>
		void Emplace(std::string& name, Args&&... args) requires std::constructible_from<T, Args...>
		{
			m_nodes[name] = new T(args...);
		}

		void Remove(const std::string& name);

		void Clear();

		auto begin() { return m_nodes.begin(); }
		auto end()   { return m_nodes.end();   }

		auto begin() const { return m_nodes.begin(); }
		auto end()   const { return m_nodes.end();   }

		virtual Node* Clone() const override;
	};
}