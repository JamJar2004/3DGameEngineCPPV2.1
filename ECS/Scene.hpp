#pragma once

#include <unordered_map>
#include <map>
#include <Reflection.hpp>

#include "EntityComponentManager.hpp"
#include "Entity.hpp"

namespace ECS
{
	class EntityCollection;

	template<std::derived_from<BaseComponent>... ComponentTypes>
	class SceneView;

	class Scene
	{
	private:
		EntityComponentManager m_manager;

		uint8_t* m_dataBuffer;
		size_t   m_dataBufferCapacity;
		size_t   m_dataBufferSize;

		std::unordered_map<std::string, Variable> m_variables;
	public:
		Scene(size_t maxEntities, size_t dataBufferCapacity = 1024);

		~Scene();

		size_t MaxEntityCount() const { return m_manager.MaxEntityCount(); }

		DynamicPointer Get(const std::string& name)
		{
			auto it = m_variables.find(name);
			DEBUG_ASSERT(it != m_variables.end(), "Variable of name " << name << " not found.");
			return DynamicPointer(m_dataBuffer, it->second);
		}

		const DynamicPointer Get(const std::string& name) const
		{
			auto it = m_variables.find(name);
			DEBUG_ASSERT(it != m_variables.end(), "Variable of name " << name << " not found.");
			return DynamicPointer(m_dataBuffer, it->second);
		}

		template<typename T>
		T& TryGet(const std::string& name, const T& defaultValue = T())
		{
			auto it = m_variables.find(name);
			if(it == m_variables.end())
			{
				m_variables[name] = Variable(name, Type::Get<T>(), m_dataBufferSize);
				T* address = new(m_dataBuffer + m_dataBufferSize) T(defaultValue);
				m_dataBufferSize += sizeof(T);
				return *address;
			}

			return DynamicPointer(m_dataBuffer, it->second);
		}

		template<typename T>
		const T& TryGet(const std::string& name, const T& defaultValue = T()) const
		{
			auto it = m_variables.find(name);
			if(it == m_variables.end())
				Set(name, defaultValue);

			return DynamicPointer(m_dataBuffer, it->second);
		}

		template<typename T>
		T& Get(const std::string& name) { return Get(name); }

		template<typename T>
		const T& Get(const std::string& name) const { return Get(name); }

		DynamicPointer operator[](const std::string& name) { return Get(name); }

		const DynamicPointer operator[](const std::string& name) const { return Get(name); }

		template<typename T>
		void Set(const std::string& name, const T& value) requires std::copy_constructible<T>
		{
			auto it = m_variables.find(name);

			if(it == m_variables.end())
			{
				m_variables[name] = Variable(name, Type::Get<T>(), m_dataBufferSize);
				new(m_dataBuffer + m_dataBufferSize) T(value);
				m_dataBufferSize += sizeof(T);
			}
			else
			{
				Variable variable = it->second;
				T* dest = (T*)(m_dataBuffer + variable.Offset);
				*dest = value;
			}
		}

		//void Start();
		//void Update(float delta);
		//void Render(Camera camera);

		//Entity CreateEntity() { return Entity(&m_manager, m_manager.CreateEntity()); }

		template<std::convertible_to<const BaseComponent&>... Types>
		Entity CreateEntity(Types&&... components)
		{
			Entity result(&m_manager, m_manager.CreateEntity());
			result.AddComponents(std::forward<Types>(components)...);
			return result;
		}

		//template<std::derived_from<BaseComponent>... Types>
		//Camera CreateCamera(const Transformation& transformation, const Projection& projection, Types&&... components)
		//{
		//	Camera result(&m_manager, m_manager.CreateEntity());
		//	result.AddComponent(transformation);
		//	result.AddComponent(projection);
		//	result.AddComponents(std::forward<Types>(components)...);
		//	return result;
		//}

		void DeleteEntity(Entity entity);

		Entity GetEntity(size_t index);

		template<std::derived_from<BaseComponent>... ComponentTypes>
		SceneView<ComponentTypes...> View();

		EntityCollection GetEntities();
	};

	class SceneViewIterator;

	template<std::derived_from<BaseComponent>... ComponentTypes>
	class SceneView
	{
	private:
		EntityComponentManager& m_manager;
		std::bitset<64> m_type;
	public:
		SceneView(EntityComponentManager& manager) : m_manager(manager)
		{
			size_t typeIDs[] = { Component<ComponentTypes>::Type.ID... };
			for(size_t i = 0; i < sizeof...(ComponentTypes); i++)
				m_type.set(typeIDs[i], true);
		}

		SceneViewIterator begin();
		SceneViewIterator end();
	};

	class SceneViewIterator
	{
	private:
		EntityIterator  m_iterator;
		std::bitset<64> m_type;
	public:
		SceneViewIterator(const EntityIterator& iterator, std::bitset<64> type) : 
			m_iterator(iterator), m_type(type) 
		{
		}

		SceneViewIterator& operator++() 
		{
			do
			{
				if(m_iterator.IsEnd())
					break;

				++m_iterator;
			}
			while((m_iterator->Type & m_type) != m_type);

			return *this;
		}

		Entity operator*() const { return Entity(m_iterator.m_manager, *m_iterator); }

		bool operator==(const SceneViewIterator& other) const { return m_iterator == other.m_iterator; }
	};

	template<std::derived_from<BaseComponent> ...ComponentTypes>
	inline SceneViewIterator SceneView<ComponentTypes...>::begin()
	{
		EntityIterator begin = m_manager.begin();

		while((begin->Type & m_type) != m_type)
		{
			if(begin == m_manager.end())
				break;

			++begin;
		}

		return SceneViewIterator(begin, m_type);
	}

	template<std::derived_from<BaseComponent> ...ComponentTypes>
	inline SceneViewIterator SceneView<ComponentTypes...>::end()
	{
		EntityIterator end = m_manager.end();
		return SceneViewIterator(end, m_type);
	}

	class EntityCollection
	{
	private:
		EntityComponentManager* m_manager;
	public:
		EntityCollection(EntityComponentManager* manager) : m_manager(manager) {}

		SceneViewIterator begin() { return SceneViewIterator(m_manager->begin(), std::bitset<64>(1)); }
		SceneViewIterator end()   { return SceneViewIterator(m_manager->end(), std::bitset<64>(1));   }
	};

	//template<std::derived_from<UpdaterSystem> T, typename... Args>
	//inline void Scene::Enable(Priority priority, Args&&... args) requires std::constructible_from<T, Args...>
	//{
	//	SystemType type = GetSystemType<T>();

	//	auto it = m_updatePriorities.find(type);
	//	if(it == m_updatePriorities.end())
	//	{
	//		m_updatePriorities[type] = priority;
	//		std::shared_ptr<UpdaterSystem> system = std::make_shared<T>(args...);
	//		m_updaterSystems[priority] = system;
	//		system->OnCreate(*this);
	//	}
	//	else
	//		m_updaterSystems[it->second]->m_isEnabled = true;
	//}

	//template<std::derived_from<UpdaterSystem> T>
	//inline void Scene::Disable()
	//{
	//	SystemType type = GetSystemType<T>();

	//	auto it = m_updatePriorities.find(type);
	//	if(it == m_updatePriorities.end())
	//		return;

	//	Priority priority = it->second;
	//	m_updaterSystems[priority]->m_isEnabled = false;
	//}

	//template<std::derived_from<RendererSystem> T, typename... Args>
	//inline void Scene::Enable(Priority priority, Args&&... args) requires std::constructible_from<T, Args...>
	//{
	//	SystemType type = GetSystemType<T>();

	//	auto it = m_renderPriorities.find(type);
	//	if(it == m_renderPriorities.end())
	//	{
	//		m_renderPriorities[type] = priority;
	//		std::shared_ptr<RendererSystem> system = std::make_shared<T>(args...);
	//		m_rendererSystems[priority] = system;
	//		system->OnCreate(*this);
	//	}
	//	else
	//		m_rendererSystems[it->second]->m_isEnabled = true;
	//}

	//template<std::derived_from<RendererSystem> T>
	//inline void Scene::Disable()
	//{
	//	SystemType type = GetSystemType<T>();

	//	auto it = m_renderPriorities.find(type);
	//	if(it == m_renderPriorities.end())
	//		return;

	//	Priority priority = it->second;
	//	m_rendererSystems[priority]->m_isEnabled = false;
	//}

	template<std::derived_from<BaseComponent> ...ComponentTypes>
	inline SceneView<ComponentTypes...> Scene::View()
	{
		return SceneView<ComponentTypes...>(m_manager);
	}
}