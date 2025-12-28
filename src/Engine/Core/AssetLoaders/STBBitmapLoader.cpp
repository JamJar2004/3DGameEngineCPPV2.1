#include "STBBitmapLoader.hpp"

#include <iostream>
#include <stb_image.h>

BitmapBaseHandle STBBitmapLoader::Load(Scene& scene, std::filesystem::path path) const
{
    int width = 0;
    int height = 0;
    uint8_t* pixels = stbi_load(path.c_str(), &width, &height, nullptr, static_cast<int>(Format.ChannelCount));

    if (!pixels)
    {
        std::cerr << "Failed to load: " << path.string() << " Reason: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    BitmapBaseHandle result = Format.CreateBitmap(width, height, 1);
    memcpy(result->GetPixels(), pixels, result->PixelCount * Format.Size);
    stbi_image_free(pixels);
    return result;
}
