#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/AudioSourceComponent.hpp"

class AudioUpdaterSystem : public UpdaterSystem
{
public:
	void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		for(auto [entity, transformation, audioSourceComponent ] : scene.View<Transformation, AudioSourceComponent>())
		{
			glm::vec3 sourceToCameraVector = transformation.Position - scene.PrimaryCamera.GetTransformation().Position;
			float distance = glm::length(sourceToCameraVector);

			float attenuation = audioSourceComponent.SoundAttenuation.Constant + audioSourceComponent.SoundAttenuation.Linear * distance + audioSourceComponent.SoundAttenuation.Exponent * distance * distance + 0.0001f;

			glm::vec3 sourceToCameraVectorNormalized = glm::normalize(sourceToCameraVector);

			glm::vec3 leftVector = glm::rotate(scene.PrimaryCamera.GetTransformation().Rotation, glm::vec3(-1, 0, 0));

			float left  = glm::dot(leftVector, sourceToCameraVectorNormalized) * 0.5f + 0.5f;
			float right = 1.0f - left;

			audioSourceComponent.AudioSource->SetBalance(left, right);
			audioSourceComponent.AudioSource->SetVolume((1.0f / attenuation) * audioSourceComponent.Volume);
		}
	}
};