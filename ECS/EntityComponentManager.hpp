#pragma once

#include <unordered_map>
#include <bitset>
#include <memory>
#include <functional>

#include <Reflection.hpp>

#include "Component.hpp"
#include "Event.hpp"

class SharedDynamicRef
{
private:
	void*   m_address;
	Type*   m_type;
	size_t* m_refCount;

	void AddRef()
	{
		if(m_address)
		{
			++(*m_refCount);
		}
	}

	void RemoveRef()
	{
		if(m_address)
		{
			--(*m_refCount);
			if(*m_refCount == 0)
			{
				m_type->Destructor(m_address);
				free(m_address);
				delete m_refCount;
			}
		}
	}

	SharedDynamicRef(void* address, Type* type) : m_address(address), m_type(type), m_refCount(new size_t(1)) 
	{
	}
public:
	SharedDynamicRef(std::nullptr_t) : m_address(nullptr), m_type(nullptr), m_refCount(nullptr) {}

	SharedDynamicRef(const SharedDynamicRef& other) : m_address(other.m_address), m_type(other.m_type), m_refCount(other.m_refCount)
	{
		AddRef();
	}

	~SharedDynamicRef()
	{
		RemoveRef();
	}

	template<typename T>
	T& Dereference() const { return *(T*)m_address; }

	SharedDynamicRef& operator=(const SharedDynamicRef& other)
	{
		RemoveRef();

		m_address  = other.m_address;
		m_type     = other.m_type;
		m_refCount = other.m_refCount;

		AddRef();
	}

	template<typename T, typename... Args>
	inline static SharedDynamicRef Create(Args&&... args) requires std::constructible_from<T, Args...>
	{
		Type* type = Type::Get<T>();
		void* address = malloc(type->Size);
		new(address) T(args...);
		return SharedDynamicRef(address, type);
	}
};

namespace ECS
{
	class EntityData
	{
	public:
		EntityData(size_t version, bool isVisible = true) :
			Version(version),
			IsVisible(isVisible) {}

		std::unordered_map<const ComponentType*, size_t> ComponentIndices;
		std::bitset<64> Type;
		size_t Version;
		bool IsVisible;
	};

	class EntityHandle
	{
	private:
		size_t m_index;
		size_t m_version;

		EntityHandle(size_t index, size_t version) : m_index(index), m_version(version) {}
	public:
		static const EntityHandle Null;

		size_t Index()   const { return m_index;   }
		size_t Version() const { return m_version; }

		friend class EntityComponentManager;
		friend class EntityIterator;
	};

	class EntityIterator;

	class EntityComponentManager
	{
	private:
		std::unordered_map<const ComponentType*, std::shared_ptr<BaseComponentArray>> m_components;

		EntityData* m_entities;
		size_t m_maxEntities;
		size_t m_end;
		size_t m_nextVersion;
		std::unordered_set<size_t> m_freeEntitySlots;

		std::unordered_map<size_t, std::unordered_map<size_t, std::vector<SharedDynamicRef>>> m_subscribedEvents;

		      EntityData* GetEntity(EntityHandle handle);
		const EntityData* GetEntity(EntityHandle handle) const;

		std::shared_ptr<BaseComponentArray> TryGetComponentArray(const ComponentType& type) const;
		std::shared_ptr<BaseComponentArray> GetComponentArray(const ComponentType& type);

		template<std::derived_from<BaseComponent> T>
		std::shared_ptr<ComponentArray<T>> GetComponentArray();
	public:
		EntityComponentManager(size_t maxEntities) : m_entities((EntityData*)malloc(sizeof(EntityData) * maxEntities)), m_maxEntities(maxEntities), m_end(0), m_nextVersion(1) {}

		~EntityComponentManager()
		{
			for(size_t i = 0; i < m_end; i++)
			{
				if(m_freeEntitySlots.contains(i))
					continue;

				(m_entities + i)->~EntityData();
			}
		}

		size_t MaxEntityCount() const { return m_maxEntities; }

		EntityHandle CreateEntity();

		void DeleteEntity(EntityHandle handle);

		bool& GetEntityVisibility(EntityHandle handle);

		EntityHandle GetEntityFromIndex(size_t index);

		bool IsHandleValid(EntityHandle handle);

		void AddComponent(EntityHandle handle, const BaseComponent& component);

		template<std::derived_from<BaseComponent> T>
		T& AddComponent(EntityHandle handle, const T& value) requires std::copy_constructible<T>;

		template<std::derived_from<BaseComponent> T, typename... Args>
		T& AddComponent(EntityHandle handle, Args&&... args) requires std::constructible_from<T, Args...>;

		BaseComponent* GetComponent(EntityHandle handle, const ComponentType& type) const;

		void RemoveComponent(EntityHandle handle, const ComponentType& type);

		bool ContainsComponent(EntityHandle handle, const ComponentType& type) const;

		void SubscribeEvent(EntityHandle handle, size_t eventId, const SharedDynamicRef& handler);
			
		void UnsubscribeEvent(EntityHandle handle, size_t eventId);

		template<typename... Args>
		void TriggerEvent(EntityHandle handle, size_t eventId, Args&&... args);

		EntityIterator begin();
		EntityIterator end();

		friend class EntityIterator;
		friend class ComponentIterator;
		friend class ConstComponentIterator;
		friend class ComponentCollection;
	};

	template<std::derived_from<BaseComponent> T>
	inline std::shared_ptr<ComponentArray<T>> EntityComponentManager::GetComponentArray()
	{
		std::shared_ptr<BaseComponentArray> result = TryGetComponentArray(Component<T>::Type);
		if(!result)
		{
			result = std::make_shared<ComponentArray<T>>(m_maxEntities);
			m_components[&Component<T>::Type] = result;
		}
		return std::dynamic_pointer_cast<ComponentArray<T>>(result);
	}

	template<std::derived_from<BaseComponent> T>
	inline T& EntityComponentManager::AddComponent(EntityHandle handle, const T& value) requires std::copy_constructible<T>
	{
		const ComponentType& type = Component<T>::Type;

		EntityData* entity = GetEntity(handle);
		DEBUG_ASSERT(!entity->Type.test(type.ID), "Entity already has component of type " << type << ".");

		std::shared_ptr<ComponentArray<T>> componentArray = GetComponentArray<T>();
		size_t componentIndex = componentArray->Add(value);

		entity->ComponentIndices[&type] = componentIndex;
		entity->Type.set(type.ID, true);

		return *(T*)componentArray->Get(componentIndex);
	}

	template<std::derived_from<BaseComponent> T, typename... Args>
	inline T& EntityComponentManager::AddComponent(EntityHandle handle, Args&& ...args) requires std::constructible_from<T, Args...>
	{
		const ComponentType& type = Component<T>::Type;

		EntityData* entity = GetEntity(handle);
		DEBUG_ASSERT(!entity->Type.test(type.ID), "Entity already has component of type " << type << ".");

		std::shared_ptr<ComponentArray<T>> componentArray = GetComponentArray<T>();
		size_t componentIndex = componentArray->Add(std::forward<Args>(args)...);

		entity->ComponentIndices[&type] = componentIndex;
		entity->Type.set(type.ID, true);

		return *(T*)componentArray->Get(componentIndex);
	}

	template<typename ...Args>
	inline void EntityComponentManager::TriggerEvent(EntityHandle handle, size_t eventId, Args&&... args)
	{
		auto entityIt = m_subscribedEvents.find(handle.m_version);
		if(entityIt != m_subscribedEvents.end())
		{
			auto eventIt = entityIt->second.find(eventId);
			if(eventIt != entityIt->second.end())
			{
				for(auto& eventHandler : eventIt->second)
				{
					auto& handler = eventHandler.Dereference<std::function<void(Args...)>>();
					handler(std::forward<Args>(args)...);
				}
			}
		}
	};

	class EntityIterator
	{
	private:
		EntityComponentManager* m_manager;
		size_t m_index;
		bool m_isEnd;

		void FindNext()
		{
			while(m_manager->m_freeEntitySlots.contains(m_index))
			{
				if(m_index == m_manager->m_end)
					return;

				++m_index;
			}
		}

		void FindPrev()
		{
			while(m_manager->m_freeEntitySlots.contains(m_index))
			{
				if(m_index == 0)
					return;

				--m_index;
			}
		}
	public:
		EntityIterator(EntityComponentManager* manager, size_t index) : m_manager(manager), m_index(index), m_isEnd(false)
		{
			if(m_index == m_manager->m_end)
				m_isEnd = true;
		}

		bool IsEnd() const { return m_isEnd; }

		EntityIterator& operator++()
		{
			if(m_index == m_manager->m_end)
			{
				m_isEnd = true;
				return *this;
			}

			++m_index;
			FindNext();
			return *this;
		}

		EntityIterator& operator--()
		{
			--m_index;
			FindPrev();
			return *this;
		}

		EntityHandle operator*() const 
		{
			EntityData* entity = m_manager->m_entities + m_index;
			return EntityHandle(m_index, entity->Version);
		}

		EntityData* operator->() const { return m_manager->m_entities + m_index; }

		bool operator==(const EntityIterator& other) const { return m_manager == other.m_manager && m_index == other.m_index; }

		friend class SceneViewIterator;
	};
}