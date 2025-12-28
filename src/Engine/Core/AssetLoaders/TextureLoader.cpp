#include "TextureLoader.hpp"

#include "../Scene.hpp"

TextureAtlasHandle TextureLoader::Load(Scene& scene, std::filesystem::path path) const
{
    auto bitmap = BitmapLoader->Load(scene, path);
    return scene.CreateTextureAtlas(bitmap, MinFilter, MagFilter, WrappingMode);
}
