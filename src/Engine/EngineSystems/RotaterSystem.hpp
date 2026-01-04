#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/RotaterComponent.hpp"

struct RotaterSystem : public UpdaterSystem
{
	virtual void OnStart(Scene& scene) override {}

	virtual void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		for(ECS::Entity entity : scene.View<Transformation, RotaterComponent>())
		{
			      Transformation&   transformation   = entity.GetComponent<Transformation>();
			const RotaterComponent& rotaterComponent = entity.GetComponent<RotaterComponent>();

			transformation.Rotate(rotaterComponent.Axis, rotaterComponent.Speed * delta);
		}
	}
};