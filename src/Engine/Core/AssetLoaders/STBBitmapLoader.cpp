#include "STBBitmapLoader.hpp"

#include <iostream>
#include <stb_image.h>

BitmapBaseHandle STBBitmapLoader::Load(const std::filesystem::path& path, PixelFormat* desiredFormat) const
{
    int width = 0;
    int height = 0;
    uint8_t* pixels = stbi_load(path.c_str(), &width, &height, nullptr, static_cast<int>(desiredFormat->ChannelCount));

    if (!pixels)
    {
        std::cerr << "Failed to load: " << path.string() << " Reason: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    BitmapBaseHandle result = desiredFormat->CreateBitmap(width, height, 1);
    memcpy(result->GetPixels(), pixels, result->PixelCount * desiredFormat->PixelType->Size);
    stbi_image_free(pixels);
    return result;
}
