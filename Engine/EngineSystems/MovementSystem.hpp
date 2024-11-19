#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/MovementComponent.hpp"

struct MovementSystem : public UpdaterSystem
{
	virtual void OnStart(Scene& scene) override {}

	virtual void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		for(ECS::Entity entity : scene.View<Transformation, MovementComponent>())
		{
			      Transformation&    transformation    = entity.GetComponent<Transformation>();
			const MovementComponent& movementComponent = entity.GetComponent<MovementComponent>();

			transformation.Position += movementComponent.Direction * movementComponent.Speed * delta;
		}
	}
};