#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/FollowerComponent.hpp"

class FollowerSystem : public UpdaterSystem
{
public:
	FollowerSystem() {}

	virtual void OnStart(Scene& scene) override {}

	virtual void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		for(ECS::Entity entity : scene.View<Transformation, FollowerComponent>())
		{
			const FollowerComponent& followerComponent = entity.GetComponent<FollowerComponent>();
			if(!followerComponent.Target.ContainsComponent<Transformation>())
				continue;

			Transformation& destTransformation = entity.GetComponent<Transformation>();
			
			const Transformation& sourceTransformation = followerComponent.Target.GetComponent<Transformation>();

			if(followerComponent.FollowPosition)
				destTransformation.Position = sourceTransformation.Position + glm::rotate(sourceTransformation.Rotation, followerComponent.PositionOffset);

			if(followerComponent.FollowRotation)
				destTransformation.Rotation = sourceTransformation.Rotation;
		}
	}
};