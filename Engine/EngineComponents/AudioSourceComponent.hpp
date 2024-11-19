#pragma once

#include <ECS/Component.hpp>

#include "../Audio/AudioSource.hpp"
#include "LightComponent.hpp"

class AudioSourceComponent : public ECS::Component<AudioSourceComponent>
{
public:
	AudioSourceComponent(AudioSourceHandle audioSource, Attenuation attenuation) : AudioSource(audioSource), Volume(1.0f), Attenuation(attenuation) {}
	
	AudioSourceHandle AudioSource;

	float       Volume;
	Attenuation Attenuation;
};