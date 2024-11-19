#pragma once

#include "Node.hpp"

namespace Json
{
	class Value : public Node
	{
	public:
		virtual       void* GetValue()       = 0;
		virtual const void* GetValue() const = 0;
	};

	template<typename T>
	class TypedValue : public Value
	{
	public:
		TypedValue(T value) : LiteralValue(value) {}

		TypedValue(const TypedValue<T>* other) : LiteralValue(other->LiteralValue) {}

		T LiteralValue;

		Node* Clone() const override { return new TypedValue<T>(this); }

		      void* GetValue()       override { return &LiteralValue; }
		const void* GetValue() const override { return &LiteralValue; }
	};
}