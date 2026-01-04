#include "Entity.hpp"

#include "Scene.hpp"

namespace ECS
{
    Entity Entity::Null(nullptr, EntityHandle::Null);

    bool Entity::IsVisible() const { return m_manager->GetEntityVisibility(m_handle); }
    bool Entity::IsValid()   const { return m_manager->IsHandleValid(m_handle);       }

    void Entity::Show() const { m_manager->GetEntityVisibility(m_handle) = true;  }
    void Entity::Hide() const { m_manager->GetEntityVisibility(m_handle) = false; }

    void Entity::Delete() const
    {
        m_manager->DeleteEntity(m_handle); 
        m_handle = EntityHandle::Null;
    }
}