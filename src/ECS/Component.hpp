#pragma once

#include <unordered_set>
#include <concepts>
#include <memory>

#include <Common.hpp>

namespace ECS
{
	class BaseComponentArray;

	using CreateArrayFunc = std::shared_ptr<BaseComponentArray>(*)(size_t);

	class ComponentType
	{
	public:
		const uint8_t         ID;
		const std::string     Name;
		const CreateArrayFunc CreateArray;

		bool operator==(const ComponentType& other) const { return ID == other.ID; }

		template<typename T>
		friend class Component;
	private:
		static uint8_t s_lastID;

		ComponentType(const std::string& name, CreateArrayFunc createArrayFunc) :
			ID(s_lastID++),
			Name(name),
			CreateArray(createArrayFunc) {}
	};

	std::ostream& operator<<(std::ostream& stream, const ComponentType& type);

	class ComponentBase
	{
	public:
		virtual ~ComponentBase() = default;

		[[nodiscard]] virtual const ComponentType& GetType() const = 0;
	};

	template<typename TSelf>
	class Component : public ComponentBase
	{
	public:
		static ComponentType Type;

		static std::shared_ptr<BaseComponentArray> CreateArray(size_t capacity);

		[[nodiscard]] const ComponentType& GetType() const override { return Type; }
	};

	template<typename TSelf>
	ComponentType Component<TSelf>::Type(typeid(TSelf).name(), CreateArray);

	template<std::derived_from<ComponentBase> T>
	class ComponentArray;

	class BaseComponentArray
	{
	public:
		virtual ~BaseComponentArray() = default;

		[[nodiscard]] virtual const ComponentType& GetElementType() const = 0;

		              virtual       ComponentBase* Get(size_t index)       = 0;
		[[nodiscard]] virtual const ComponentBase* Get(size_t index) const = 0;

		virtual size_t Add(const ComponentBase& component) = 0;
		virtual void   Remove(size_t index) = 0;

		template<std::derived_from<ComponentBase> T>
		ComponentArray<T>& As();

		template<std::derived_from<ComponentBase> T>
		const ComponentArray<T>& As() const;
	};

	template<std::derived_from<ComponentBase> T>
	class ComponentArray : public BaseComponentArray
	{
	private:
		T* const m_data;

		const size_t m_capacity;
		
		size_t m_end;

		std::unordered_set<size_t> m_freeIndices;
	public:
		explicit ComponentArray(size_t capacity) :
			m_data(static_cast<T*>(malloc(sizeof(T) * capacity))), m_capacity(capacity), m_end(0U) {}

		~ComponentArray() override
		{
			for(size_t i = 0; i < m_end; i++)
			{
				if(m_freeIndices.contains(i))
					continue;

				(m_data + i)->~T();
			}

			free(m_data);
		}

		const ComponentType& GetElementType() const override { return Component<T>::Type; }

		ComponentBase* Get(size_t index) override
		{ 
			DEBUG_ASSERT(index < m_end && !m_freeIndices.contains(index), "Invalid index.");
			return m_data + index;
		}

		const ComponentBase* Get(size_t index) const override
		{
			DEBUG_ASSERT(index < m_end && !m_freeIndices.contains(index), "Invalid index.");
			return m_data + index;
		}

		size_t Add(const ComponentBase& component) override
		{
			DEBUG_ASSERT(component.GetType() == Component<T>::Type, "Component type mismatch.");
			return Add(static_cast<const T&>(component));
		}

		size_t Add(const T& value) requires std::copy_constructible<T>
		{
			size_t result;

			auto it = m_freeIndices.begin();
			if(it == m_freeIndices.end())
			{
				DEBUG_ASSERT(m_end < m_capacity, "Exceeded maximum number of entities.");
				result = m_end++;
			}
			else
			{
				result = *it;
				m_freeIndices.erase(it);
			}

			new(m_data + result) T(value);
			return result;
		}

		template<typename... Args>
		size_t Add(Args&&... args) requires std::constructible_from<T, Args...>
		{
			size_t result;

			auto it = m_freeIndices.begin();
			if(it == m_freeIndices.end())
			{
				DEBUG_ASSERT(m_end < m_capacity, "Exceeded maximum number of entities.");
				result = m_end++;
			}
			else
			{
				result = *it;
				m_freeIndices.erase(it);
			}

			new(m_data + result) T(args...);
			return result;
		}

		void Remove(size_t index) override
		{
			DEBUG_ASSERT(index < m_end && !m_freeIndices.contains(index), "Invalid index.");

			if(index == m_end - 1)
				--m_end;
			else
				m_freeIndices.insert(index);

			(m_data + index)->~T();
		}
	};

	template<typename T>
	std::shared_ptr<BaseComponentArray> Component<T>::CreateArray(size_t capacity)
	{
		return std::make_shared<ComponentArray<T>>(capacity);
	}

	template<std::derived_from<ComponentBase> T>
	ComponentArray<T>& BaseComponentArray::As()
	{
		DEBUG_ASSERT(GetElementType() == Component<T>::Type, "Invalid cast from " << GetElementType() << " to " << Component<T>::Type << ".");
		return *static_cast<ComponentArray<T>*>(this);
	}

	template<std::derived_from<ComponentBase> T>
	const ComponentArray<T>& BaseComponentArray::As() const
	{
		DEBUG_ASSERT(GetElementType() == Component<T>::Type, "Invalid cast from " << GetElementType() << " to " << Component<T>::Type << ".");
		return *static_cast<const ComponentArray<T>*>(this);
	}
}