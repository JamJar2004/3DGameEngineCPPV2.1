#pragma once

#include <ECS/Entity.hpp>

#include "../EngineComponents/Transformation.hpp"
#include "../EngineComponents/Projection.hpp"

struct Camera : ECS::Entity
{
	explicit Camera(Entity source = Null) : Entity(source) {}

	Transformation& GetTransformation() const { return GetComponent<Transformation>(); }

	Projection& GetProjection() const { return GetComponent<Projection>(); }

	glm::mat4 GetViewMatrix() const
	{
		const glm::mat4 positionMatrix = glm::translate(glm::identity<glm::mat4>(), -GetTransformation().GetTransformedPosition());
		const glm::mat4 rotationMatrix = glm::toMat4(glm::conjugate(GetTransformation().GetTransformedRotation()));
		const glm::mat4    scaleMatrix = glm::scale(glm::identity<glm::mat4>(), 1.0f / GetTransformation().GetTransformedScale());

		return rotationMatrix * positionMatrix * scaleMatrix;
	}

	glm::mat4 GetViewProjection() const
	{
		return GetProjection().Matrix * GetViewMatrix();
	}
};