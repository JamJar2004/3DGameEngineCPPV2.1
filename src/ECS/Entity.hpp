#pragma once

#include <unordered_map>
#include <bitset>
#include <memory>

#include "EntityComponentManager.hpp"
#include "Event.hpp"

namespace ECS
{
	class Scene;

	class ComponentIterator
	{
	private:
		using Iterator = std::unordered_map<const ComponentType*, size_t>::iterator;

		EntityComponentManager* m_manager;
		Iterator m_iterator;
	public:
		ComponentIterator(EntityComponentManager* manager, Iterator iterator) : 
			m_manager(manager), m_iterator(iterator) {}

		ComponentIterator& operator++()
		{
			++m_iterator;
			return *this;
		}

		ComponentBase& operator*() const 
		{ 
			auto componentArray = m_manager->GetComponentArray(*m_iterator->first);
			return *componentArray->Get(m_iterator->second);
		}

		bool operator==(const ComponentIterator& other) const { return m_iterator == other.m_iterator; }
	};

	class ConstComponentIterator
	{
	private:
		using Iterator = std::unordered_map<const ComponentType*, size_t>::iterator;

		EntityComponentManager* m_manager;
		Iterator m_iterator;
	public:
		ConstComponentIterator(EntityComponentManager* manager, Iterator iterator) :
			m_manager(manager), m_iterator(iterator) {}

		ConstComponentIterator& operator++()
		{
			++m_iterator;
			return *this;
		}

		const ComponentBase& operator*() const 
		{ 
			auto componentArray = m_manager->GetComponentArray(*m_iterator->first);
			return *componentArray->Get(m_iterator->second);
		}

		bool operator==(const ConstComponentIterator& other) const { return m_iterator == other.m_iterator; }
	};

	class ComponentCollection
	{
	private:
		EntityComponentManager* m_manager;
		EntityHandle            m_handle;
	public:
		ComponentCollection(EntityComponentManager* manager, EntityHandle handle) :
			m_manager(manager), m_handle(handle)
		{
			
		}

		ComponentIterator begin() { return {m_manager, m_manager->GetEntity(m_handle)->ComponentIndices.begin()}; }
		ComponentIterator end()   { return {m_manager, m_manager->GetEntity(m_handle)->ComponentIndices.end()};   }
		
		[[nodiscard]] ConstComponentIterator begin() const { return {m_manager, m_manager->GetEntity(m_handle)->ComponentIndices.begin()}; }
		[[nodiscard]] ConstComponentIterator end()   const { return {m_manager, m_manager->GetEntity(m_handle)->ComponentIndices.end()};   }
	};



	class Entity
	{
	private:
		        EntityComponentManager* m_manager;
		mutable EntityHandle            m_handle;
	protected:
		Entity(EntityComponentManager* manager, EntityHandle handle) : m_manager(manager), m_handle(handle)  {}
	public:
		static Entity Null;

		Entity() : m_manager(nullptr), m_handle(EntityHandle::Null) {}

		size_t GetIndex()   const { return m_handle.Index();   }
		size_t GetVersion() const { return m_handle.Version(); }

		      ComponentCollection GetComponents()       { return {m_manager, m_handle}; }
		const ComponentCollection GetComponents() const { return ComponentCollection(m_manager, m_handle); }

		bool IsVisible() const;
		bool IsValid() const;

		template<std::derived_from<ComponentBase> T>
		void AddComponent(const T& component) const requires std::copy_constructible<T>;

		template<std::derived_from<ComponentBase> T, typename... Args>
		T& AddComponent(Args&&... args) const requires std::constructible_from<T, Args...>;

		void AddComponents() const {}

		template<std::derived_from<ComponentBase> T>
		void AddComponents(T&& component) const { AddComponent(component); }

		template<std::convertible_to<const ComponentBase&> First, std::convertible_to<const ComponentBase&>... Rest>
		void AddComponents(First&& first, Rest&&... rest) const;

		template<std::derived_from<ComponentBase> T>
		T& GetComponent() const;

		template<std::derived_from<ComponentBase> T>
		void RemoveComponent() const;

		template<std::derived_from<ComponentBase> T>
		bool ContainsComponent() const;

		void Show() const;
		void Hide() const;

		void Delete() const;
		
		template<typename T, typename... Args>
		void SubscribeEvent(const EntityEvent<Args...>& event, T invokable) const requires std::invocable<T, Args...>;
		
		template<typename... Args>
		void UnsubscribeEvent(const EntityEvent<Args...>& event) const;

		template<typename... Args>
		void TriggerEvent(const EntityEvent<Args...>& event, Args&&... args) const;

		bool operator==(const Entity& other) const { return m_manager == other.m_manager && m_handle.Version() == other.m_handle.Version(); }

		friend class Scene;

		template<std::derived_from<ComponentBase>... ComponentTypes>
		friend class SceneViewIterator;

		template<std::derived_from<ComponentBase>... ComponentTypes>
		friend class SceneRawViewIterator;
	};

	template<typename... Args>
	void EntityEvent<Args...>::operator()(Entity& entity, Args... args)
	{
		entity.TriggerEvent<Args...>(*this, std::forward<Args>(args)...);
	}

	template<std::derived_from<ComponentBase> T>
	void Entity::AddComponent(const T& component) const requires std::copy_constructible<T>
	{
		m_manager->AddComponent(m_handle, component);
	}

	template<std::derived_from<ComponentBase> T, typename ...Args>
	T& Entity::AddComponent(Args&&... args) const requires std::constructible_from<T, Args...>
	{
		return m_manager->AddComponent<T>(m_handle, args...);
	}

	template<std::convertible_to<const ComponentBase&> First, std::convertible_to<const ComponentBase&>... Rest>
	void Entity::AddComponents(First&& first, Rest&&... rest) const
	{
		AddComponent(first);
		AddComponents(std::forward<Rest>(rest)...);
	}

	template<std::derived_from<ComponentBase> T>
	inline T& Entity::GetComponent() const
	{
		ComponentBase* result = m_manager->GetComponent(m_handle, Component<T>::Type);
		return *(T*)result;
	}

	template<std::derived_from<ComponentBase> T>
	inline void Entity::RemoveComponent() const
	{
		m_manager->RemoveComponent(m_handle, Component<T>::Type);
	}

	template<std::derived_from<ComponentBase> T>
	inline bool Entity::ContainsComponent() const
	{
		return m_manager->ContainsComponent(m_handle, Component<T>::Type);
	}

	template<typename T, typename ...Args>
	inline void Entity::SubscribeEvent(const EntityEvent<Args...>& event, T invokable) const requires std::invocable<T, Args...>
	{
		m_manager->SubscribeEvent(m_handle, event.Id, SharedDynamicRef::Create<std::function<void(Args...)>>(invokable));
	}

	template<typename ...Args>
	inline void Entity::UnsubscribeEvent(const EntityEvent<Args...>& event) const
	{
		m_manager->UnsubscribeEvent(m_handle, event.Id);
	}

	template<typename... Args>
	inline void Entity::TriggerEvent(const EntityEvent<Args...>& event, Args&&... args) const
	{
		m_manager->TriggerEvent(m_handle, event.Id, std::forward<Args>(args)...);
	}
}