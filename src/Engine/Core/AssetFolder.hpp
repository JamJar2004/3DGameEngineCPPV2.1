#pragma once

#include <filesystem>
#include <string_view>

class Scene;

template<typename TAsset>
struct AssetLoader
{
    virtual ~AssetLoader() = default;

    virtual TAsset Load(Scene& scene, std::filesystem::path path) const = 0;
};

class AssetFolder
{
public:
    explicit AssetFolder(Scene& scene, const std::filesystem::path& path) : Scene(scene), Path(path) {}

    Scene& Scene;
    std::filesystem::path Path;

    template<typename TAsset>
    TAsset Load(std::string_view fileName, const AssetLoader<TAsset>& loader)
    {
        return loader.Load(Path / fileName);
    }

    AssetFolder Navigate(std::string_view name) const { return AssetFolder(Scene, Path / name); }
};