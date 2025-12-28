#pragma once

#include "Engine/Core/AssetFolder.hpp"
#include "Engine/Rendering/TextureAtlas.hpp"

class STBBitmapLoader final : public AssetLoader<BitmapBaseHandle>
{
public:
    explicit STBBitmapLoader(const PixelFormat& format) : Format(format) {}

    PixelFormat Format;

    BitmapBaseHandle Load(Scene& scene, std::filesystem::path path) const override;
};
