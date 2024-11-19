#pragma once

#include <string>
#include <concepts>

#include "Common.hpp"

using CopyConstructor = void(*)(const void*, void*);
using Destructor = void(*)(void*);

class Type
{
private:
	inline static size_t s_nextID;

	template<typename T>
	class Storage
	{
	public:
		static Type* const s_type;
	};

	Type(size_t id, const std::string& name, size_t size, const void* defaultValue, CopyConstructor copyConstructor, Destructor destructor) : 
		ID(id), Name(name), Size(size), DefaultValue(defaultValue), CopyConstructor(copyConstructor), Destructor(destructor) {}
public:
	Type(const Type&) = delete;

	Type& operator=(const Type&) = delete;

	template<typename T>
	inline static Type* Get() 
	{
		Type* type = Storage<T>::s_type;
		return type;
	}

	const size_t ID;

	const std::string Name;
	const size_t      Size;

	const void* const DefaultValue;

	const CopyConstructor CopyConstructor;
	const Destructor      Destructor;
};


template<typename T>
static void Copy(const void* source, void* dest)
{
	DEBUG_ASSERT(false, "Type does not have copy constructor.");
}

template<std::copy_constructible T>
static void Copy(const void* source, void* dest)
{
	new(dest) T(*(T*)source);
}

template<typename T>
static void Destroy(void* address)
{
	((T*)address)->~T();
}

template<typename T>
static T* GetDefaultValue() { return nullptr; }

template<std::default_initializable T>
static T* GetDefaultValue() { return new T(); }

template<typename T>
Type* const Type::Storage<T>::s_type(new Type(s_nextID++, typeid(T).name(), sizeof(T), GetDefaultValue<T>(), Copy<T>, Destroy<T>));

class Variable
{	
public:
	Variable() : Name(""), Type(nullptr), Offset(0) {}

	Variable(const std::string& name, Type* type, size_t offset) :
		Name(name), Type(type), Offset(offset) {}

	std::string Name;
	Type*       Type;
	size_t      Offset;
};

class DynamicPointer
{
private:
	uint8_t* const  m_data;
	const Variable& m_variable;
public:
	DynamicPointer(uint8_t* data, const Variable& variable) : m_data(data), m_variable(variable) {}

	template<typename T>
	DynamicPointer& operator=(const T& other) requires std::assignable_from<T, T>
	{ 
		DEBUG_ASSERT(Type::Get<T>() == m_variable.Type, "Cannot convert from " << Type::Get<T>().Name << " to " << m_variable.Type << ".");
		T* dest = (T*)(m_data + m_variable.Offset);
		*dest = other;
		return *this; 
	}

	template<typename T>
	operator T&()
	{
		DEBUG_ASSERT(Type::Get<T>() == m_variable.Type, "Cannot convert from " << m_variable.Type->Name << " to " << Type::Get<T>()->Name << ".");
		T* source = (T*)(m_data + m_variable.Offset);
		return *source;
	}

	template<typename T>
	operator const T&() const
	{
		DEBUG_ASSERT(Type::Get<T>() == m_variable.Type, "Cannot convert from " << m_variable.Type->Name << " to " << Type::Get<T>()->Name << ".");
		T* source = (T*)(m_data + m_variable.Offset);
		return *source;
	}

	friend class Scene;
};

class Dynamic
{
private:
	void* m_address;
	Type* m_type;

	Dynamic(void* address, Type* type) : m_address(address), m_type(type) {}
public:
	template<std::copy_constructible T>
	Dynamic(const T& other) : m_address(malloc(sizeof(T))), m_type(Type::Get<T>())
	{
		new(m_address) T(other);
	}

	Dynamic(const Dynamic& other) : m_address(malloc(other.m_type->Size)), m_type(other.m_type) 
	{
		m_type->CopyConstructor(other.m_address, m_address);
	}

	Type* GetType() const { return m_type; }

	template<typename T, typename... Args>
	static Dynamic Create(Args... args) requires std::constructible_from<T, Args...>
	{
		void* address = malloc(sizeof(T));
		new(address) T(args...);
		return Dynamic(address, Type::Get<T>());
	}

	~Dynamic()
	{
		m_type->Destructor(m_address);
		free(m_address);
	}

	template<typename T>
	T& UncheckedCast() { return *(T*)m_address; }

	template<typename T>
	const T& UncheckedCast() const { return *(T*)m_address; }
};