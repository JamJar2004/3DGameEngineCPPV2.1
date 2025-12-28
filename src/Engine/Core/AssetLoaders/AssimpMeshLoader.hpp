#pragma once

#include "Engine/Core/AssetFolder.hpp"
#include "Engine/Rendering/Mesh.hpp"

class AssimpMeshLoader final : public AssetLoader<MeshHandle>
{
public:
    explicit AssimpMeshLoader(bool smoothNormals) : SmoothNormals(smoothNormals) {}

    bool SmoothNormals;

    MeshHandle Load(Scene& scene, std::filesystem::path path) const override;
};
