#pragma once

#include "Engine/Core/AssetFolder.hpp"

struct AssimpModelLoader final : ModelLoaderBase
{
    [[nodiscard]] Model Load(const std::filesystem::path& path, bool smoothNormals) const override;
};
