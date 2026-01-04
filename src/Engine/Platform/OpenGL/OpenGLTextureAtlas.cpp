#include "OpenGLTextureAtlas.hpp"

GL::InternalImageFormat GetInternalFormat(InternalImageFormat format)
{
	switch(format)
	{
		case InternalImageFormat::R8:
			return GL::InternalImageFormat::R8;
		case InternalImageFormat::R16:
			return GL::InternalImageFormat::R16;
		case InternalImageFormat::R32:
			return GL::InternalImageFormat::R32UI;
		case InternalImageFormat::R16F:
			return GL::InternalImageFormat::R16F;
		case InternalImageFormat::R32F:
			return GL::InternalImageFormat::R32F;
		case InternalImageFormat::RG8:
			return GL::InternalImageFormat::RG8;
		case InternalImageFormat::RG16:
			return GL::InternalImageFormat::RG16;
		case InternalImageFormat::RG32:
			return GL::InternalImageFormat::RG32UI;
		case InternalImageFormat::RG16F:
			return GL::InternalImageFormat::RG16F;
		case InternalImageFormat::RG32F:
			return GL::InternalImageFormat::RG32F;
		case InternalImageFormat::RGB8:
			return GL::InternalImageFormat::RGB8;
		case InternalImageFormat::RGB16:
			return GL::InternalImageFormat::RGB16;
		case InternalImageFormat::RGB32:
			return GL::InternalImageFormat::RGB32UI;
		case InternalImageFormat::RGB16F:
			return GL::InternalImageFormat::RGB16F;
		case InternalImageFormat::RGB32F:
			return GL::InternalImageFormat::RGB32F;
		case InternalImageFormat::RGBA8:
			return GL::InternalImageFormat::RGBA8;
		case InternalImageFormat::RGBA16:
			return GL::InternalImageFormat::RGBA16;
		case InternalImageFormat::RGBA32:
			return GL::InternalImageFormat::RGBA32UI;
		case InternalImageFormat::RGBA16F:
			return GL::InternalImageFormat::RGBA16F;
		case InternalImageFormat::RGBA32F:
			return GL::InternalImageFormat::RGBA32F;
	}

	return GL::InternalImageFormat::RGBA8;
}

GL::ImageFormat GetFormat(ImageFormat format)
{
	switch(format)
	{
		case ImageFormat::R:
			return GL::ImageFormat::R;
		case ImageFormat::RG:
			return GL::ImageFormat::RG;
		case ImageFormat::RGB:
			return GL::ImageFormat::RGB;
		case ImageFormat::RGBA:
			return GL::ImageFormat::RGBA;
	}
	return GL::ImageFormat::RGBA;
}

GL::MinFilterMode GetMinFilterMode(MinFilterMode filterMode)
{
	switch(filterMode)
	{
		case MinFilterMode::Nearest:
			return GL::MinFilterMode::Nearest;
		case MinFilterMode::Linear:
			return GL::MinFilterMode::Linear;
		case MinFilterMode::NearestMipmapNearest:
			return GL::MinFilterMode::NearestMipmapNearest;
		case MinFilterMode::NearestMipmapLinear:
			return GL::MinFilterMode::NearestMipmapLinear;
		case MinFilterMode::LinearMipmapNearest:
			return GL::MinFilterMode::LinearMipmapNearest;
		case MinFilterMode::LinearMipmapLinear:
			return GL::MinFilterMode::LinearMipmapLinear;
	}
	
	return GL::MinFilterMode::Linear;
}

GL::MagFilterMode GetMagFilterMode(MagFilterMode filterMode)
{
	switch(filterMode)
	{
		case MagFilterMode::Nearest:
			return GL::MagFilterMode::Nearest;
		case MagFilterMode::Linear:
			return GL::MagFilterMode::Linear;
	}

	return GL::MagFilterMode::Linear;
}

GL::TextureWrappingMode GetWrappingMode(TextureWrappingMode wrappingMode)
{
	switch(wrappingMode)
	{
		case TextureWrappingMode::Tiled:
			return GL::TextureWrappingMode::Repeat;
		case TextureWrappingMode::TiledMirrored:
			return GL::TextureWrappingMode::MirroredRepeat;
		case TextureWrappingMode::ClampedToEdge:
			return GL::TextureWrappingMode::ClampToEdge;
		case TextureWrappingMode::ClampedToBorder:
			return GL::TextureWrappingMode::ClampToBorder;
		case TextureWrappingMode::ClampedToEdgeMirrored:
			return GL::TextureWrappingMode::MirrorClampToEdge;
	}

	return GL::TextureWrappingMode::Repeat;
}

void OpenGLTextureAtlas::Bind(uint32_t slot)
{
	m_texture.Bind(slot);
}

void OpenGLTextureCube::Bind(uint32_t slot)
{
	m_texture.Bind(slot);
}
