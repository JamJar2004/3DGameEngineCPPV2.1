#include "Object.hpp"

#include <ranges>

namespace Json
{
	Object::Object(const Object* other)
	{
		for(const auto& [name, value] : other->m_nodes)
		{
			m_nodes[name] = value->Clone();
		}
	}

	Object::~Object()
	{
		Clear();
	}

	Node* Object::Get(std::string_view name)
	{
		return m_nodes[std::string(name)];
	}

	const Node* Object::Get(std::string_view name) const
	{
		auto it = m_nodes.find(std::string(name));
		if(it == m_nodes.end())
		{
			return nullptr;
		}
		return it->second;
	}

	void Object::Add(std::string_view name, const Node* value)
	{
		m_nodes[std::string(name)] = value->Clone();
	}

	void Object::Remove(std::string_view name)
	{
		const auto it = m_nodes.find(std::string(name));
		delete it->second;
		m_nodes.erase(it);
	}

	void Object::Clear()
	{
		for(const auto& value: m_nodes | std::views::values)
		{
			delete value;
		}

		m_nodes.clear();
	}

	Node* Object::Clone() const { return new Object(this); }
}
