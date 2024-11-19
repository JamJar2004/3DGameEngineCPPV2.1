#pragma once

#include "../../Rendering/Texture.hpp"
#include "../../Rendering/RenderDevice.hpp"

#include "OpenGLCommon.hpp"

#include <OpenGLRenderer/Texture.hpp>

GL::InternalFormat GetInternalFormat(InternalFormat internalFormat);
GL::Format GetFormat(Format format, InternalFormat internalFormat);

GL::MinFilterMode GetMinFilterMode(MinFilterMode filterMode);
GL::MagFilterMode GetMagFilterMode(MagFilterMode filterMode);

GL::WrappingMode GetWrappingMode(WrappingMode wrappingMode);

class OpenGLTexture : public Texture
{
private:
	GL::Texture2D m_texture;
public:
	OpenGLTexture(const BaseBitmap& image, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode) :
		Texture(image.Width, image.Height),
		m_texture(image.Width, image.Height, image.GetPixels(), GetInternalFormat(image.Format.BufferInternalFormat), GetFormat(image.Format.BufferFormat, image.Format.BufferInternalFormat), GetElementType(image.Format.BufferElementType), GetMinFilterMode(minFilter), GetMagFilterMode(magFilter), GetWrappingMode(wrappingMode)) {}

	virtual void Bind(uint32_t slot) override;
};

class OpenGLTextureCube : public Texture
{
private:
	GL::TextureCube m_texture;
public:
	OpenGLTextureCube(const BaseBitmap& image, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode) :
		Texture(image.Width, image.Height),
		m_texture(image.Width, image.Height, uint32_t(image.Format.ChannelCount), image.GetPixels(), GetInternalFormat(image.Format.BufferInternalFormat), GetFormat(image.Format.BufferFormat, image.Format.BufferInternalFormat), GetElementType(image.Format.BufferElementType), GetMinFilterMode(minFilter), GetMagFilterMode(magFilter), GetWrappingMode(wrappingMode)) {}

	virtual void Bind(uint32_t slot) override;
};
