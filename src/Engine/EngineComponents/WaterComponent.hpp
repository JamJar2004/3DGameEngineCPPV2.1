#pragma once

#include <ECS/Component.hpp>
#include <Engine/Rendering/Material.hpp>

class WaterComponent : public ECS::Component<WaterComponent>
{
public:
	WaterComponent(MaterialHandle material, float waveSpeed = 0.01f) :
		Material(material), WaveSpeed(waveSpeed) {}

	MaterialHandle Material;

	float WaveSpeed;
};