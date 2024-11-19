#pragma once

#include <glm/glm.hpp>

class AABB
{
public:
	AABB(const glm::vec3& minimum, const glm::vec3& maximum) : Minimum(minimum), Maximum(maximum)
	{
		Minimum.x = std::min(Minimum.x, Maximum.x);
		Minimum.y = std::min(Minimum.y, Maximum.y);
		Minimum.z = std::min(Minimum.z, Maximum.z);

		Maximum.x = std::max(Maximum.x, Maximum.x);
		Maximum.y = std::max(Maximum.y, Maximum.y);
		Maximum.z = std::max(Maximum.z, Maximum.z);
	}

	glm::vec3 Minimum;
	glm::vec3 Maximum;

	glm::vec3 Size() const { return Maximum - Minimum; }

	bool Contains(const AABB& other) const
	{
		return Minimum.x < other.Minimum.x && Maximum.x >= other.Maximum.x &&
			   Minimum.y < other.Minimum.y && Maximum.y >= other.Maximum.y &&
			   Minimum.z < other.Minimum.z && Maximum.z >= other.Maximum.z;
	}

	bool Intersects(const AABB& other)
	{

	}
};