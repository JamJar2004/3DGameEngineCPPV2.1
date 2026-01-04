#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/MouseLookComponent.hpp"

struct MouseLookSystem final : UpdaterSystem
{
	void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		for(auto [ entity, transformation, mouseLookComponent ] : scene.View<Transformation, MouseLookComponent>())
		{
			auto leftButtonState = mouse.GetButtonState(MouseButton::Left);

			const glm::vec2 deltaPos = mouse.GetPosition();

			const bool rotY = deltaPos.x != 0 && mouseLookComponent.CanYaw;
			const bool rotX = deltaPos.y != 0 && mouseLookComponent.CanPitch;

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