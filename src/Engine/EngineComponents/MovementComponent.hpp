#pragma once

#include <glm/glm.hpp>

#include <ECS/Component.hpp>

struct MovementComponent : public ECS::Component<MovementComponent>
{
	MovementComponent(const glm::vec3& direction, float speed) : Direction(glm::normalize(direction)), Speed(speed) {}

	glm::vec3 Direction;
	float     Speed;
};