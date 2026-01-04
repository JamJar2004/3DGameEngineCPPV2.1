#pragma once

#include <ECS/Component.hpp>

struct MouseLookComponent : public ECS::Component<MouseLookComponent>
{
	MouseLookComponent(float sensitivity) : Sensitivity(sensitivity), CanPitch(true), CanYaw(true) {}

	float Sensitivity;

	bool CanPitch;
	bool CanYaw;
};
