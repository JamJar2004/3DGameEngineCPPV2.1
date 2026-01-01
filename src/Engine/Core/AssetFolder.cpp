#include "AssetFolder.hpp"

#include <fstream>

#include "Scene.hpp"

BitmapBaseHandle AssetFolder::LoadBitmap(std::string_view fileName, const PixelFormat& desiredFormat) const
{
    return m_assetLoaders.BitmapLoader->Load(Path / fileName, desiredFormat);
}

MeshHandle AssetFolder::LoadTerrain(std::string_view fileName) const
{
    return m_assetLoaders.SourceScene.CreateTerrain(HeightMap(LoadBitmap<uint8_t>(fileName)));
}

MeshHandle AssetFolder::LoadMesh(std::string_view fileName, bool smoothNormals) const
{
    return m_assetLoaders.SourceScene.CreateMesh(m_assetLoaders.ModelLoader->Load(Path / fileName, smoothNormals));
}

TextureAtlasHandle AssetFolder::LoadTextureAtlas(
    std::string_view fileName, const PixelFormat& format,
    MinFilterMode minFilter, MagFilterMode magFilter,
    TextureWrappingMode wrappingMode) const
{
    return m_assetLoaders.SourceScene.CreateTextureAtlas(*LoadBitmap(fileName, format),
                                                         minFilter, magFilter, wrappingMode);
}

ShaderHandle AssetFolder::LoadShader(std::string_view fileName) const
{
    std::ifstream stream;
    stream.open(Path / fileName);

    std::string line;

    std::stringstream shaderSourceCode;
    while(std::getline(stream, line))
    {
        shaderSourceCode << line << '\n';
    }

    stream.close();
    return m_assetLoaders.SourceScene.CreateShader(shaderSourceCode.str());
}

FontHandle AssetFolder::LoadFont(std::string_view fileName) const
{
    const auto& atlas = *LoadBitmap<glm::u8vec4>(std::string(fileName) + ".png");
    std::ifstream jsonStream;
    jsonStream.open(Path / fileName);
    auto result = m_assetLoaders.SourceScene.CreateFont(atlas, jsonStream);
    jsonStream.close();
    return result;
}


SoundHandle AssetFolder::LoadSound(std::string_view fileName) const
{
    return m_assetLoaders.SoundLoader->Load(Path / fileName);
}

MusicHandle AssetFolder::LoadMusic(std::string_view fileName) const
{
    return m_assetLoaders.MusicLoader->Load(Path / fileName);
}

