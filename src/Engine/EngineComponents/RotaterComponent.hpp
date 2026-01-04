#pragma once

#include <glm/glm.hpp>

#include <ECS/Component.hpp>

struct RotaterComponent : public ECS::Component<RotaterComponent>
{
	RotaterComponent(const glm::vec3& axis, float speed) : Axis(glm::normalize(axis)), Speed(speed) {}

	glm::vec3 Axis;
	float     Speed;
};