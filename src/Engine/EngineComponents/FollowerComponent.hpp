#pragma once

#include <ECS/Entity.hpp>

class FollowerComponent : public ECS::Component<FollowerComponent>
{
public:
	FollowerComponent(ECS::Entity target, const glm::vec3& positionOffset, bool followPosition = true, bool followRotation = false) : 
		Target(target), PositionOffset(positionOffset), FollowPosition(followPosition), FollowRotation(followRotation) {}

	ECS::Entity Target;

	glm::vec3 PositionOffset;

	bool FollowPosition;
	bool FollowRotation;
};