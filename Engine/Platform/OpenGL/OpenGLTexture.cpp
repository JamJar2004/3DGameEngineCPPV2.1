#include "OpenGLTexture.hpp"

static GL::InternalFormat GetInternalFormat(InternalFormat format)
{
	switch(format)
	{
		case InternalFormat::R8:
			return GL::InternalFormat::R8;
		case InternalFormat::R16:
			return GL::InternalFormat::R16;
		case InternalFormat::R32:
			return GL::InternalFormat::R32UI;
		case InternalFormat::R16F:
			return GL::InternalFormat::R16F;
		case InternalFormat::R32F:
			return GL::InternalFormat::R32F;
		case InternalFormat::RG8:
			return GL::InternalFormat::RG8;
		case InternalFormat::RG16:
			return GL::InternalFormat::RG16;
		case InternalFormat::RG32:
			return GL::InternalFormat::RG32UI;
		case InternalFormat::RG16F:
			return GL::InternalFormat::RG16F;
		case InternalFormat::RG32F:
			return GL::InternalFormat::RG32F;
		case InternalFormat::RGB8:
			return GL::InternalFormat::RGB8;
		case InternalFormat::RGB16:
			return GL::InternalFormat::RGB16;
		case InternalFormat::RGB32:
			return GL::InternalFormat::RGB32UI;
		case InternalFormat::RGB16F:
			return GL::InternalFormat::RGB16F;
		case InternalFormat::RGB32F:
			return GL::InternalFormat::RGB32F;
		case InternalFormat::RGBA8:
			return GL::InternalFormat::RGBA8;
		case InternalFormat::RGBA16:
			return GL::InternalFormat::RGBA16;
		case InternalFormat::RGBA32:
			return GL::InternalFormat::RGBA32UI;
		case InternalFormat::RGBA16F:
			return GL::InternalFormat::RGBA16F;
		case InternalFormat::RGBA32F:
			return GL::InternalFormat::RGBA32F;
	}

	return GL::InternalFormat::RGBA8;
}

static GL::Format GetFormat(Format format, InternalFormat internalFormat)
{
	bool isInteger =
		internalFormat == InternalFormat::R8 ||
		internalFormat == InternalFormat::R16 ||
		internalFormat == InternalFormat::R32 ||
		internalFormat == InternalFormat::RG8 ||
		internalFormat == InternalFormat::RG16 ||
		internalFormat == InternalFormat::RG32 ||
		internalFormat == InternalFormat::RGB8 ||
		internalFormat == InternalFormat::RGB16 ||
		internalFormat == InternalFormat::RGB32 ||
		internalFormat == InternalFormat::RGBA8 ||
		internalFormat == InternalFormat::RGBA16 ||
		internalFormat == InternalFormat::RGBA32;

	if(!isInteger)
	{
		switch(format)
		{
			case Format::R:
				return GL::Format::R;
			case Format::RG:
				return GL::Format::RG;
			case Format::RGB:
				return GL::Format::RGB;
			case Format::RGBA:
				return GL::Format::RGBA;
		}
	}
	else
	{
		switch(format)
		{
			case Format::R:
				return GL::Format::RInteger;
			case Format::RG:
				return GL::Format::RGInteger;
			case Format::RGB:
				return GL::Format::RGBInteger;
			case Format::RGBA:
				return GL::Format::RGBAInteger;
		}
	}
	return GL::Format::RGBA;
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

GL::WrappingMode GetWrappingMode(WrappingMode wrappingMode)
{
	switch(wrappingMode)
	{
		case WrappingMode::Tiled:
			return GL::WrappingMode::Repeat;
		case WrappingMode::TiledMirrored:
			return GL::WrappingMode::MirroredRepeat;
		case WrappingMode::ClampedToEdge:
			return GL::WrappingMode::ClampToEdge;
		case WrappingMode::ClampedToBorder:
			return GL::WrappingMode::ClampToBorder;
		case WrappingMode::ClampedToEdgeMirrored:
			return GL::WrappingMode::MirrorClampToEdge;
	}

	return GL::WrappingMode::Repeat;
}

void OpenGLTexture::Bind(uint32_t slot)
{
	m_texture.Bind(slot);
}

void OpenGLTextureCube::Bind(uint32_t slot)
{
	m_texture.Bind(slot);
}
