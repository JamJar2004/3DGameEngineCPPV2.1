#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/MouseLookComponent.hpp"

struct MouseLookSystem : public UpdaterSystem
{
	virtual void OnStart(Scene& scene) override {}

	virtual void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		for(ECS::Entity entity : scene.View<Transformation, MouseLookComponent>())
		{
			      Transformation&     transformation     = entity.GetComponent<Transformation>();
			const MouseLookComponent& mouseLookComponent = entity.GetComponent<MouseLookComponent>();

			auto leftButtonState = mouse.GetButtonState(MouseButton::Left);

			glm::vec2 deltaPos = mouse.GetPosition();

			bool rotY = (deltaPos.x != 0) && mouseLookComponent.CanYaw;
			bool rotX = (deltaPos.y != 0) && mouseLookComponent.CanPitch;

			if(rotY)
			{
				transformation.Rotate(glm::vec3(0, 1, 0), -deltaPos.x * mouseLookComponent.Sensitivity * 180.0f);
			}

			if(rotX)
			{
				transformation.Rotate(glm::rotate(transformation.Rotation, glm::vec3(1, 0, 0)), deltaPos.y * mouseLookComponent.Sensitivity * 180.0f);
			}

			if(rotY || rotX)
			{
				mouse.SetPosition(glm::vec2(0, 0));
			}
			
		}
	}
};