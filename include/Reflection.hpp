#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <concepts>

#include "Common.hpp"

using DefaultConstructorFunc = void*(*)();
using CopyConstructorFunc = void(*)(const void*, void*);
using DestructorFunc = void(*)(void*);


class TypeInfo
{
public:
	TypeInfo(const TypeInfo&) = delete;

	TypeInfo& operator=(const TypeInfo&) = delete;

	template<typename T>
	static TypeInfo* Get()
	{
		TypeInfo* type = Storage<T>::s_type;
		return type;
	}

	const size_t ID;

	const std::string Name;
	const size_t      Size;

	const DefaultConstructorFunc DefaultConstructor;
	const CopyConstructorFunc    CopyConstructor;
	const DestructorFunc         Destructor;

private:
	inline static size_t s_nextID;

	template<typename T>
	class Storage
	{
	public:
		static TypeInfo* const s_type;
	};

	TypeInfo(size_t id, std::string_view name, size_t size, DefaultConstructorFunc defaultConstructor, CopyConstructorFunc copyConstructor, DestructorFunc destructor) :
		ID(id), Name(name), Size(size), DefaultConstructor(defaultConstructor), CopyConstructor(copyConstructor), Destructor(destructor) {}
};

template<typename T>
static void* CreateDefaultValue()
{
	if constexpr (std::default_initializable<T>)
	{
		return new T();
	}
	else
	{
		return nullptr;
	}
}

template<typename T>
static void Copy(const void* source, void* dest)
{
	if constexpr(std::copy_constructible<T>)
	{
		new(dest) T(*(T*)source);
	}
	else
	{
		DEBUG_ASSERT(false, "Type does not have copy constructor.");
	}
}

template<typename T>
static void Destroy(void* address)
{
	((T*)address)->~T();
}

template<typename T>
inline TypeInfo* const TypeInfo::Storage<T>::s_type(new TypeInfo(s_nextID++, std::string_view(typeid(T).name()), sizeof(T), CreateDefaultValue<T>, Copy<T>, Destroy<T>));

class Variable
{	
public:
	Variable() : Name(""), Type(nullptr), Offset(0) {}

	Variable(const std::string& name, TypeInfo* type, size_t offset) :
		Name(name), Type(type), Offset(offset) {}

	std::string Name;
	TypeInfo*   Type;
	size_t      Offset;
};

class DynamicPointer
{

public:
	DynamicPointer(uint8_t* data, const Variable& variable) : m_data(data), m_variable(variable) {}

	template<typename T>
	DynamicPointer& operator=(const T& other) requires std::assignable_from<T, T>
	{ 
		DEBUG_ASSERT(TypeInfo::Get<T>() == m_variable.Type, "Cannot convert from " << TypeInfo::Get<T>().Name << " to " << m_variable.Type << ".");
		auto dest = static_cast<T*>(m_data + m_variable.Offset);
		*dest = other;
		return *this; 
	}

	template<typename T>
	operator T&()
	{
		DEBUG_ASSERT(TypeInfo::Get<T>() == m_variable.Type, "Cannot convert from " << m_variable.Type->Name << " to " << TypeInfo::Get<T>()->Name << ".");
		T* source = static_cast<T*>(m_data + m_variable.Offset);
		return *source;
	}

	template<typename T>
	operator const T&() const
	{
		DEBUG_ASSERT(TypeInfo::Get<T>() == m_variable.Type, "Cannot convert from " << m_variable.Type->Name << " to " << TypeInfo::Get<T>()->Name << ".");
		T* source = static_cast<T*>(m_data + m_variable.Offset);
		return *source;
	}

	friend class Scene;
private:
	uint8_t* const  m_data;
	const Variable& m_variable;
};

class Dynamic
{
public:
	template<std::copy_constructible T>
	Dynamic(const T& other) : m_address(malloc(sizeof(T))), m_type(TypeInfo::Get<T>())
	{
		new(m_address) T(other);
	}

	Dynamic(const Dynamic& other) : m_address(malloc(other.m_type->Size)), m_type(other.m_type) 
	{
		m_type->CopyConstructor(other.m_address, m_address);
	}

	[[nodiscard]] TypeInfo* GetType() const { return m_type; }

	template<typename T, typename... Args>
	static Dynamic Create(Args... args) requires std::constructible_from<T, Args...>
	{
		void* address = malloc(sizeof(T));
		new(address) T(args...);
		return Dynamic(address, TypeInfo::Get<T>());
	}

	~Dynamic()
	{
		m_type->Destructor(m_address);
		free(m_address);
	}

	template<typename T>
	T& UncheckedCast() { return *static_cast<T*>(m_address); }

	template<typename T>
	const T& UncheckedCast() const { return *static_cast<T*>(m_address); }
private:
	void* m_address;
	TypeInfo* m_type;

	Dynamic(void* address, TypeInfo* type) : m_address(address), m_type(type) {}
};