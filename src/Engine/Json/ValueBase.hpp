#pragma once

#include "Node.hpp"

namespace Json
{
	class ValueBase : public Node
	{
	public:
		              virtual       void* GetValue()       = 0;
		[[nodiscard]] virtual const void* GetValue() const = 0;
	};

	template<typename TValue>
	class Value final : public ValueBase
	{
	public:
		explicit Value(const TValue& value) : LiteralValue(value) {}

		explicit Value(const Value* other) : LiteralValue(other->LiteralValue) {}

		TValue LiteralValue;

		[[nodiscard]] Node* Clone() const override { return new Value(this); }

		                    void* GetValue()       override { return &LiteralValue; }
		[[nodiscard]] const void* GetValue() const override { return &LiteralValue; }
	};
}