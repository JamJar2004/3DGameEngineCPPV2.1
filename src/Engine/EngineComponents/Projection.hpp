#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <ECS/Component.hpp>

struct Projection : public ECS::Component<Projection>
{
	Projection(float fieldOfView, float aspectRatio, float zNear, float zFar) :
		Matrix(glm::perspective(glm::radians(fieldOfView), aspectRatio, zNear, zFar)) {}

	Projection(float left, float right, float bottom, float top, float back, float front) :
		Matrix(glm::ortho(left, right, bottom, top, back, front)) {}

	Projection(const glm::mat4& matrix) : Matrix(matrix) {}

	glm::mat4 Matrix;
};