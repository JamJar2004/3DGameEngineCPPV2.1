#pragma once

#include "SpecularMaterial.hpp"

struct NormalMappedMaterial : SpecularMaterial
{
    NormalMappedMaterial(
        const glm::vec3& color,
        float specularIntensity,
        const glm::vec2& tilingFactor,
        const glm::vec2& textureRegion,
        const glm::vec2& normalMapRegion) :
            SpecularMaterial(color, specularIntensity, tilingFactor, textureRegion),
            NormalMapRegion(normalMapRegion) {}

    glm::vec2 NormalMapRegion;
};
