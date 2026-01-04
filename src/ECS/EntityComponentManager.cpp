#include "EntityComponentManager.hpp"

namespace ECS
{
	const EntityHandle EntityHandle::Null(0, 0);

	EntityData* EntityComponentManager::GetEntity(EntityHandle handle)
	{
		DEBUG_ASSERT(handle.m_index < m_end && !m_freeEntitySlots.contains(handle.m_index), "Invalid entity handle.");
		EntityData* result = m_entities + handle.m_index;
		DEBUG_ASSERT(result->Version == handle.m_version, "Invalid entity handle.");
		return result;
	}

	const EntityData* EntityComponentManager::GetEntity(EntityHandle handle) const
	{
		DEBUG_ASSERT(handle.m_index < m_end && !m_freeEntitySlots.contains(handle.m_index), "Invalid entity handle.");
		EntityData* result = m_entities + handle.m_index;
		DEBUG_ASSERT(result->Version == handle.m_version, "Invalid entity handle.");
		return result;
	}

	std::shared_ptr<BaseComponentArray> EntityComponentManager::TryGetComponentArray(const ComponentType& type) const
	{
		auto it = m_components.find(&type);
		if(it == m_components.end())
			return nullptr;

		return it->second;
	}

	std::shared_ptr<BaseComponentArray> EntityComponentManager::GetComponentArray(const ComponentType& type)
	{
		std::shared_ptr<BaseComponentArray> result = TryGetComponentArray(type);
		if(!result)
		{
			result = type.CreateArray(m_maximumEntityCount);
			m_components[&type] = result;
		}
		return result;
	}

	EntityHandle EntityComponentManager::CreateEntity()
	{
		size_t index;

		auto it = m_freeEntitySlots.begin();
		if(it == m_freeEntitySlots.end())
		{
			index = m_end++;
		}
		else
		{
			index = *it;
			m_freeEntitySlots.erase(it);
		}

		size_t version = m_nextVersion++;

		new(m_entities + index) EntityData(version);
		return EntityHandle(index, version);
	}

	void EntityComponentManager::DeleteEntity(EntityHandle handle)
	{
		EntityData* entity = GetEntity(handle);
		
		if(handle.m_index == m_end - 1)
		{
			--m_end;
		}
		else
		{
			m_freeEntitySlots.insert(handle.m_index);
		}

		entity->~EntityData();
		m_subscribedEvents.erase(handle.Version());
	}

	bool& EntityComponentManager::GetEntityVisibility(EntityHandle handle) { return GetEntity(handle)->IsVisible; }

	EntityHandle EntityComponentManager::GetEntityFromIndex(size_t index)
	{
		//DEBUG_ASSERT(index < m_end && !m_freeEntitySlots.contains(index), "Invalid entity index.");
		EntityData* result = m_entities + index;
		return EntityHandle(index, result->Version);
	}

	bool EntityComponentManager::IsHandleValid(EntityHandle handle)
	{
		if(handle.m_index >= m_end || m_freeEntitySlots.contains(handle.m_index))
		{
			return false;
		}
		EntityData* entity = m_entities + handle.m_index;
		return entity->Version == handle.m_version;
	}

	void EntityComponentManager::AddComponent(EntityHandle handle, const ComponentBase& component)
	{
		const ComponentType& type = component.GetType();

		EntityData* entity = GetEntity(handle);
		DEBUG_ASSERT(!entity->Type.test(type.ID), "Entity already has component of type " << type << ".");

		std::shared_ptr<BaseComponentArray> componentArray = GetComponentArray(type);
		size_t componentIndex = componentArray->Add(component);

		entity->ComponentIndices[&type] = componentIndex;
		entity->Type.set(type.ID, true);
	}

	ComponentBase* EntityComponentManager::GetComponent(EntityHandle handle, const ComponentType& type) const
	{
		const EntityData* entity = GetEntity(handle);
		DEBUG_ASSERT(entity->Type.test(type.ID), "Entity doesn't have component of type " << type << ".");

		size_t index = entity->ComponentIndices.find(&type)->second;

		std::shared_ptr<BaseComponentArray> componentArray = TryGetComponentArray(type);
		DEBUG_ASSERT(componentArray, "Component array of type " << type << " does not exist");
		return componentArray->Get(index);
	}

	void EntityComponentManager::RemoveComponent(EntityHandle handle, const ComponentType& type)
	{
		EntityData* entity = GetEntity(handle);
		DEBUG_ASSERT(entity->Type.test(type.ID), "Entity doesn't have component of type " << type << ".");

		size_t index = entity->ComponentIndices[&type];

		std::shared_ptr<BaseComponentArray> componentArray = TryGetComponentArray(type);
		DEBUG_ASSERT(componentArray, "Component array of type " << type << " does not exist");
		componentArray->Remove(index);

		entity->ComponentIndices.erase(&type);
		entity->Type.set(type.ID, false);
	}

	bool EntityComponentManager::ContainsComponent(EntityHandle handle, const ComponentType& type) const
	{
		const EntityData* entity = GetEntity(handle);
		return entity->Type.test(type.ID);
	}

	void EntityComponentManager::SubscribeEvent(EntityHandle handle, size_t eventId, const SharedDynamicRef& handler)
	{
		m_subscribedEvents[handle.Version()][eventId].push_back(handler);
	}

	void EntityComponentManager::UnsubscribeEvent(EntityHandle handle, size_t eventId)
	{
		m_subscribedEvents[handle.Version()].erase(eventId);
	}

	EntityIterator EntityComponentManager::begin()
	{
		size_t index = 0;
		while(m_freeEntitySlots.contains(index))
			++index;

		return EntityIterator(this, index);
	}

	EntityIterator EntityComponentManager::end()
	{
		return EntityIterator(this, m_end);
	}
}