#pragma once

#include <array>
#include <ECS/Entity.hpp>

template<size_t MaxItemCount>
class OctTree
{
private:
	
public:

};

template<size_t MaxItemCount>
class OctTreeNode
{
private:
	std::array<ECS::Entity, MaxItemCount> m_entities;
	std::array<OctTreeNode*, 8> m_children;
public:
	OctTreeNode()
	{
		m_entities.fill(ECS::Entity::Null);
		m_children.fill(nullptr);
	}

	void Add(ECS::Entity entity)
	{
		ColliderComponent& collider = entity.GetComponent<ColliderComponent>();
	}
};