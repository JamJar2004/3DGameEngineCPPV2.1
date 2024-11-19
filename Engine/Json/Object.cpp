#include "Object.hpp"

namespace Json
{
	Object::Object(const Object* other)
	{
		for(const auto& pair : other->m_nodes)
		{
			m_nodes[pair.first] = pair.second->Clone();
		}
	}

	Object::~Object()
	{
		Clear();
	}

	Node* Object::Get(const std::string& name)
	{
		return m_nodes[name];
	}

	const Node* Object::Get(const std::string& name) const
	{
		auto it = m_nodes.find(name);
		if(it == m_nodes.end())
		{
			return nullptr;
		}
		return it->second;
	}

	void Object::Add(const std::string& name, Node* value)
	{
		m_nodes[name] = value->Clone();
	}

	void Object::Remove(const std::string& name)
	{
		auto it = m_nodes.find(name);
		delete it->second;
		m_nodes.erase(it);
	}

	void Object::Clear()
	{
		for(const auto& pair : m_nodes)
		{
			delete pair.second;
		}

		m_nodes.clear();
	}

	Node* Object::Clone() const { return new Object(this); }
}