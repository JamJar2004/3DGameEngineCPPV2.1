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
		return *this;
	}

	template<typename T, typename... Args>
	inline static SharedDynamicRef Create(Args&&... args) requires std::constructible_from<T, Args...>
	{
		TypeInfo* type = TypeInfo::Get<T>();
		void* address = malloc(type->Size);
		new(address) T(args...);
		return SharedDynamicRef(address, type);
	}
private:
	void*     m_address;
	TypeInfo* m_type;
	size_t*   m_refCount;

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

	SharedDynamicRef(void* address, TypeInfo* type) : m_address(address), m_type(type), m_refCount(new size_t(1))
	{
	}

};

namespace ECS
{
	class EntityIterator;

	class EntityData
	{
	public:
		explicit EntityData(size_t version, bool isVisible = true) :
			Version(version),
			IsVisible(isVisible) {}

		std::unordered_map<const ComponentType*, size_t> ComponentIndices;
		std::bitset<64> Type;
		size_t Version;
		bool IsVisible;
	};

	class EntityHandle
	{
	public:
		static const EntityHandle Null;

		size_t Index()   const { return m_index;   }
		size_t Version() const { return m_version; }

		friend class EntityComponentManager;
		friend class EntityIterator;
	private:
		size_t m_index;
		size_t m_version;

		EntityHandle(size_t index, size_t version) : m_index(index), m_version(version) {}
	};

	class EntityComponentManager
	{

	public:
		EntityComponentManager(size_t maximumEntityCount) :
			m_entities(static_cast<EntityData*>(malloc(sizeof(EntityData) * maximumEntityCount))),
			m_maximumEntityCount(maximumEntityCount),
			m_end(0),
			m_nextVersion(1) {}

		~EntityComponentManager()
		{
			for(size_t i = 0; i < m_end; i++)
			{
				if(m_freeEntitySlots.contains(i))
					continue;

				(m_entities + i)->~EntityData();
			}
		}

		size_t MaxEntityCount() const { return m_maximumEntityCount; }

		EntityHandle CreateEntity();

		void DeleteEntity(EntityHandle handle);

		bool& GetEntityVisibility(EntityHandle handle);

		EntityHandle GetEntityFromIndex(size_t index);

		bool IsHandleValid(EntityHandle handle);

		void AddComponent(EntityHandle handle, const ComponentBase& component);

		template<std::derived_from<ComponentBase> TComponent>
		TComponent& AddComponent(EntityHandle handle, const TComponent& value) requires std::copy_constructible<TComponent>;

		template<std::derived_from<ComponentBase> TComponent, typename... Args>
		TComponent& AddComponent(EntityHandle handle, Args&&... args) requires std::constructible_from<TComponent, Args...>;

		ComponentBase* GetComponent(EntityHandle handle, const ComponentType& type) const;

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
	private:
		std::unordered_map<const ComponentType*, std::shared_ptr<BaseComponentArray>> m_components;

		EntityData* m_entities;
		size_t m_maximumEntityCount;
		size_t m_end;
		size_t m_nextVersion;
		std::unordered_set<size_t> m_freeEntitySlots;

		std::unordered_map<size_t, std::unordered_map<size_t, std::vector<SharedDynamicRef>>> m_subscribedEvents;

		EntityData* GetEntity(EntityHandle handle);
		const EntityData* GetEntity(EntityHandle handle) const;

		std::shared_ptr<BaseComponentArray> TryGetComponentArray(const ComponentType& type) const;
		std::shared_ptr<BaseComponentArray> GetComponentArray(const ComponentType& type);

		template<std::derived_from<ComponentBase> TComponent>
		std::shared_ptr<ComponentArray<TComponent>> GetComponentArray();
	};

	template<std::derived_from<ComponentBase> TComponent>
	std::shared_ptr<ComponentArray<TComponent>> EntityComponentManager::GetComponentArray()
	{
		std::shared_ptr<BaseComponentArray> result = TryGetComponentArray(Component<TComponent>::Type);
		if(!result)
		{
			result = std::make_shared<ComponentArray<TComponent>>(m_maximumEntityCount);
			m_components[&Component<TComponent>::Type] = result;
		}
		return std::dynamic_pointer_cast<ComponentArray<TComponent>>(result);
	}

	template<std::derived_from<ComponentBase> TComponent>
	TComponent& EntityComponentManager::AddComponent(EntityHandle handle, const TComponent& value) requires std::copy_constructible<TComponent>
	{
		const ComponentType& type = Component<TComponent>::Type;

		EntityData* entity = GetEntity(handle);
		DEBUG_ASSERT(!entity->Type.test(type.ID), "Entity already has component of type " << type << ".");

		std::shared_ptr<ComponentArray<TComponent>> componentArray = GetComponentArray<TComponent>();
		size_t componentIndex = componentArray->Add(value);

		entity->ComponentIndices[&type] = componentIndex;
		entity->Type.set(type.ID, true);

		return *(TComponent*)componentArray->Get(componentIndex);
	}

	template<std::derived_from<ComponentBase> TComponent, typename... TArgs>
	TComponent& EntityComponentManager::AddComponent(EntityHandle handle, TArgs&& ...args) requires std::constructible_from<TComponent, TArgs...>
	{
		const ComponentType& type = Component<TComponent>::Type;

		EntityData* entity = GetEntity(handle);
		DEBUG_ASSERT(!entity->Type.test(type.ID), "Entity already has component of type " << type << ".");

		std::shared_ptr<ComponentArray<TComponent>> componentArray = GetComponentArray<TComponent>();
		size_t componentIndex = componentArray->Add(std::forward<TArgs>(args)...);

		entity->ComponentIndices[&type] = componentIndex;
		entity->Type.set(type.ID, true);

		return *static_cast<TComponent*>(componentArray->Get(componentIndex));
	}

	template<typename ...TArgs>
	void EntityComponentManager::TriggerEvent(EntityHandle handle, size_t eventId, TArgs&&... args)
	{
		auto entityIt = m_subscribedEvents.find(handle.m_version);
		if(entityIt != m_subscribedEvents.end())
		{
			auto eventIt = entityIt->second.find(eventId);
			if(eventIt != entityIt->second.end())
			{
				for(auto& eventHandler : eventIt->second)
				{
					auto& handler = eventHandler.Dereference<std::function<void(TArgs...)>>();
					handler(std::forward<TArgs>(args)...);
				}
			}
		}
	};

	class EntityIterator
	{
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

		template<std::derived_from<ComponentBase>... TComponents>
		friend class SceneViewIterator;

		template<std::derived_from<ComponentBase>... TComponents>
		friend class SceneRawViewIterator;
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
	};
}