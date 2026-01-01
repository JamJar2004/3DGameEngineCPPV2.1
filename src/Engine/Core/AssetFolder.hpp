#pragma once

#include <filesystem>
#include <string_view>
#include <utility>

#include "Engine/Audio/AudioSource.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/RenderContext2D.hpp"
#include "Engine/Rendering/Shader.hpp"
#include "Engine/Rendering/TextureAtlas.hpp"

class Scene;

struct ModelLoaderBase
{
    virtual ~ModelLoaderBase() = default;

    [[nodiscard]] virtual Model Load(const std::filesystem::path& path, bool smoothNormals) const = 0;
};

struct BitmapLoaderBase
{
    virtual ~BitmapLoaderBase() = default;

    [[nodiscard]] virtual BitmapBaseHandle Load(const std::filesystem::path& path, const PixelFormat& desiredFormat) const = 0;
};

struct SoundLoaderBase
{
    virtual ~SoundLoaderBase() = default;

    [[nodiscard]] virtual SoundHandle Load(const std::filesystem::path& path) const = 0;
};


struct MusicLoaderBase
{
    virtual ~MusicLoaderBase() = default;

    [[nodiscard]] virtual MusicHandle Load(const std::filesystem::path& path) const = 0;
};


struct AssetLoaders
{
    Scene& SourceScene;

    std::shared_ptr<ModelLoaderBase> ModelLoader;

    std::shared_ptr<BitmapLoaderBase> BitmapLoader;

    std::shared_ptr<SoundLoaderBase> SoundLoader;

    std::shared_ptr<MusicLoaderBase> MusicLoader;
};

class AssetFolder
{
public:
    std::filesystem::path Path;

    [[nodiscard]] AssetFolder GetParent() const
    {
        return AssetFolder(m_assetLoaders, Path.parent_path());
    }

    [[nodiscard]] AssetFolder Navigate(std::string_view name) const { return AssetFolder(m_assetLoaders, Path / name); }

    template<typename Pixel>
    BitmapHandle<Pixel> LoadBitmap(std::string_view fileName) const
    {
        const BitmapBaseHandle result = LoadBitmap(fileName, PixelFormat::GetFormat<Pixel>());
        return std::dynamic_pointer_cast<Bitmap<Pixel>>(result);
    }

    [[nodiscard]] BitmapBaseHandle LoadBitmap(std::string_view fileName, const PixelFormat& desiredFormat) const;

    [[nodiscard]] MeshHandle LoadTerrain(std::string_view fileName) const;

    [[nodiscard]] MeshHandle LoadMesh(std::string_view fileName, bool smoothNormals = true) const;

    [[nodiscard]] TextureAtlasHandle LoadTextureAtlas(std::string_view fileName, const PixelFormat& format, MinFilterMode minFilter = MinFilterMode::LinearMipmapLinear, MagFilterMode magFilter = MagFilterMode::Linear, TextureWrappingMode wrappingMode = TextureWrappingMode::Tiled) const;

    [[nodiscard]] ShaderHandle LoadShader(std::string_view fileName) const;

    [[nodiscard]] SoundHandle LoadSound(std::string_view fileName) const;

    [[nodiscard]] MusicHandle LoadMusic(std::string_view fileName) const;

    [[nodiscard]] FontHandle LoadFont(std::string_view fileName) const;
private:
    AssetLoaders& m_assetLoaders;

    explicit AssetFolder(AssetLoaders& assetLoaders, std::filesystem::path path) : m_assetLoaders(assetLoaders), Path(std::move(path)) {}
};