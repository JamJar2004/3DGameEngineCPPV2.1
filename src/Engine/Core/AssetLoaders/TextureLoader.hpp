#pragma once

#include "STBBitmapLoader.hpp"

class TextureLoader final : public AssetLoader<TextureAtlasHandle>
{
public:
    explicit TextureLoader(
        const std::shared_ptr<AssetLoader<BitmapBaseHandle>>& bitmapLoader,
        MinFilterMode minFilter = MinFilterMode::LinearMipmapLinear,
        MagFilterMode magFilter = MagFilterMode::Linear,
        TextureWrappingMode wrappingMode = TextureWrappingMode::Tiled) :
            BitmapLoader(bitmapLoader), MinFilter(minFilter), MagFilter(magFilter), WrappingMode(wrappingMode) {}

    std::shared_ptr<AssetLoader<BitmapBaseHandle>> BitmapLoader;

    MinFilterMode MinFilter;
    MagFilterMode MagFilter;
    TextureWrappingMode WrappingMode;

    TextureAtlasHandle Load(Scene& scene, std::filesystem::path path) const override;
};