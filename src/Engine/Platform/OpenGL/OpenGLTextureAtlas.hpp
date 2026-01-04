#pragma once

#include "../../Rendering/TextureAtlas.hpp"
#include "../../Rendering/RenderDevice.hpp"

#include "OpenGLCommon.hpp"

#include <OpenGLRenderer/Texture.hpp>

GL::InternalImageFormat GetInternalFormat(InternalImageFormat internalFormat);
GL::ImageFormat GetFormat(ImageFormat format);

GL::MinFilterMode GetMinFilterMode(MinFilterMode filterMode);
GL::MagFilterMode GetMagFilterMode(MagFilterMode filterMode);

GL::TextureWrappingMode GetWrappingMode(TextureWrappingMode wrappingMode);

class OpenGLTextureAtlas final : public TextureAtlas
{
public:
	OpenGLTextureAtlas(const BitmapBase& image, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) :
		TextureAtlas(image.Width, image.Height),
		m_texture(image.Width, image.Height, image.GetPixels(), GetInternalFormat(image.Format->BufferInternalFormat), GetFormat(image.Format->BufferFormat), GetElementType(image.Format->BufferElementType), GetMinFilterMode(minFilter), GetMagFilterMode(magFilter), GetWrappingMode(wrappingMode)) {}
protected:
	void Bind(uint32_t slot) override;
private:
	GL::Texture2D m_texture;
};

class OpenGLTextureCube final : public TextureAtlas
{
public:
	OpenGLTextureCube(const BitmapBase& image, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) :
		TextureAtlas(image.Width, image.Height),
		m_texture(image.Width, image.Height, static_cast<uint32_t>(image.Format->ChannelCount), image.GetPixels(), GetInternalFormat(image.Format->BufferInternalFormat), GetFormat(image.Format->BufferFormat), GetElementType(image.Format->BufferElementType), GetMinFilterMode(minFilter), GetMagFilterMode(magFilter), GetWrappingMode(wrappingMode)) {}
protected:
	void Bind(uint32_t slot) override;
private:
	GL::TextureCube m_texture;
};
