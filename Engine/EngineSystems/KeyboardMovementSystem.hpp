#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/KeyboardMovementComponent.hpp"

#include <ECS/Event.hpp>

struct KeyboardMovementSystem : public UpdaterSystem
{
	ECS::EntityEvent<Key, Transformation&, glm::vec3, glm::vec3> MoveEvent;

	virtual void OnStart(Scene& scene) {}

	virtual void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		for(ECS::Entity entity : scene.View<Transformation, KeyboardMovementComponent>())
		{
			      Transformation&            transformation    = entity.GetComponent<Transformation>();
			const KeyboardMovementComponent& movementComponent = entity.GetComponent<KeyboardMovementComponent>();

			for(auto it = movementComponent.begin(); it != movementComponent.end(); ++it)
			{
				if(keyboard.IsKeyDown(it->first))
				{
					glm::vec3 oldPosition = transformation.Position;
					transformation.Position += glm::rotate(transformation.Rotation, it->second) * delta;
					glm::vec3 newPosition = transformation.Position;

					MoveEvent(entity, it->first, transformation, oldPosition, newPosition);
				}
			}
		}
	}
};