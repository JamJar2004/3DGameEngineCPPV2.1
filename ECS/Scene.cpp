#include "Scene.hpp"

namespace ECS
{
	Scene::Scene(size_t maxEntities, size_t dataBufferCapacity) :
		m_manager(maxEntities),
		m_dataBuffer(new uint8_t[dataBufferCapacity]),
		m_dataBufferCapacity(dataBufferCapacity),
		m_dataBufferSize(0)
	{
	}

	Scene::~Scene() 
	{ 
		for(auto it = m_variables.begin(); it != m_variables.end(); ++it)
		{
			const Variable& variable = it->second;

			void* address = m_dataBuffer + variable.Offset;
			variable.Type->Destructor(address);
		}

		delete m_dataBuffer;
	}

	//void Scene::Update(float delta)
	//{
	//	OnUpdate(delta);

	//	for(auto it = m_updaterSystems.begin(); it != m_updaterSystems.end(); ++it)
	//	{
	//		std::shared_ptr<UpdaterSystem> updaterSystem = it->second;
	//		if(!updaterSystem->IsEnabled())
	//			continue;

	//		updaterSystem->Update(*this, delta);
	//	}
	//}

	//void Scene::Render(Camera camera)
	//{
	//	OnRender(camera);

	//	for(auto it = m_rendererSystems.begin(); it != m_rendererSystems.end(); ++it)
	//	{
	//		std::shared_ptr<RendererSystem> rendererSystem = it->second;
	//		if(!rendererSystem->IsEnabled())
	//			continue;

	//		rendererSystem->Render(*this, camera);
	//	}
	//}

	void Scene::DeleteEntity(Entity entity)
	{
		entity.Delete();
	}

	Entity Scene::GetEntity(size_t index)
	{
		return Entity(&m_manager, m_manager.GetEntityFromIndex(index));
	}

	EntityCollection Scene::GetEntities() { return EntityCollection(&m_manager); }
}
