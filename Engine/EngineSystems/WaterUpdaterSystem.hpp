#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/WaterComponent.hpp"

class WaterUpdaterSystem : public UpdaterSystem
{
public:
	virtual void OnStart(Scene& scene) override {}

	virtual void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse)
	{
		for(ECS::Entity entity : scene.View<WaterComponent>())
		{
			WaterComponent& waterComponent = entity.GetComponent<WaterComponent>();
			float& offset = waterComponent.Material->GetType()->Get<float>("u_offset");
			offset += waterComponent.WaveSpeed * delta, 1.0f;
			offset = fmodf(offset, 1.0f);
		}
	}
};