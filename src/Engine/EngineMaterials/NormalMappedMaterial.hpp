#pragma once

#include "SpecularMaterial.hpp"

struct NormalMappedMaterial final
{
    NormalMappedMaterial(
        const glm::vec3& color,
        float specularIntensity,
        const glm::vec2& tilingFactor) :
            Color(color),
            SpecularIntensity(specularIntensity),
            TilingFactor(tilingFactor) {}

    glm::vec3 Color;
    float     SpecularIntensity;
    glm::vec2 TilingFactor;

    MaterialTextureInfo AlbedoTexture;
    MaterialTextureInfo NormalTexture;
};
