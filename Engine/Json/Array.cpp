#include "Array.hpp"

namespace Json
{
	Array::Array(const Array* other)
	{
		m_elements.resize(other->m_elements.size());
		for(size_t i = 0; i < m_elements.size(); i++)
		{
			m_elements[i] = other->m_elements[i]->Clone();
		}
	}

	Array::~Array()
	{
		Clear();
	}

		  Node*       Array::operator[](size_t index)       { return m_elements[index]; }
	const Node* const Array::operator[](size_t index) const { return m_elements[index]; }

	void Array::Add(Node* node)
	{
		m_elements.push_back(node->Clone());
	}

	void Array::RemoveAt(size_t index)
	{
		m_elements.erase(m_elements.begin() + index);
	}

	void Array::Clear() 
	{
		for(Node* element : m_elements)
		{
			delete element;
		}

		m_elements.clear(); 
	}

	inline Node* Array::Clone() const { return new Array(this); }
}