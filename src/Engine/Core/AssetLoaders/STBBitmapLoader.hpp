#pragma once

#include "Engine/Core/AssetFolder.hpp"
#include "Engine/Rendering/TextureAtlas.hpp"

struct STBBitmapLoader final : BitmapLoaderBase
{
    [[nodiscard]] BitmapBaseHandle Load(const std::filesystem::path& path, PixelFormat* desiredFormat) const override;
};
