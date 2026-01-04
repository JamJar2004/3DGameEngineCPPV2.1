#pragma once

#include "../Core/Scene.hpp"

class SkyboxComponent : public ECS::Component<SkyboxComponent>
{
public:
	SkyboxComponent(MaterialHandle material) : Material(material) {}

	MaterialHandle Material;
};