#pragma once

#include <ECS/Entity.hpp>

#include "../EngineComponents/Transformation.hpp"
#include "../EngineComponents/Projection.hpp"

struct Camera : public ECS::Entity
{
	Camera(const ECS::Entity& other = ECS::Entity::Null) : Entity(other) {}

	Transformation& GetTransformation() const { return GetComponent<Transformation>(); }

	Projection& GetProjection() const { return GetComponent<Projection>(); }

	glm::mat4 GetViewMatrix() const
	{
		const Transformation& transformation = GetTransformation();

		glm::mat4 positionMatrix = glm::translate(glm::identity<glm::mat4>(), -transformation.GetTransformedPosition());
		glm::mat4 rotationMatrix = glm::toMat4(glm::conjugate(transformation.GetTransformedRotation()));
		glm::mat4    scaleMatrix = glm::scale(glm::identity<glm::mat4>(), 1.0f / transformation.GetTransformedScale());

		return rotationMatrix * positionMatrix * scaleMatrix;
	}

	glm::mat4 GetViewProjection() const
	{
		return GetProjection().Matrix * GetViewMatrix();
	}
};