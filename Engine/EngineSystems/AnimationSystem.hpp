#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/AnimationComponent.hpp"

template<Animatable T>
class AnimationSystem : public UpdaterSystem
{
public:
	virtual void OnStart(Scene& scene) override {}

	virtual void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		for(ECS::Entity entity : scene.View<AnimationComponent<T>>())
		{
			AnimationComponent<T>& animation = entity.GetComponent<AnimationComponent<T>>();
			if(animation.IsPlaying)
			{
				animation.Update(delta);
			}
		}
	}
};