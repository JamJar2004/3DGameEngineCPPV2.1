#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/AnimationComponent.hpp"

template<Animatable T>
class AnimationSystem final : public UpdaterSystem
{
public:
	void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		for(auto [entity, animation] : scene.View<AnimationComponent<T>>())
		{
			if(animation.IsPlaying)
			{
				animation.Update(delta);
			}
		}
	}
};