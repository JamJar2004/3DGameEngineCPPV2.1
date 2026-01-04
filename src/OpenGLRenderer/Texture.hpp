#pragma once

#include <cstdint>
#include <GL/glew.h>
#include <array>

#include "Common.hpp"

namespace GL
{
	enum class MinFilterMode
	{
		Nearest              = GL_NEAREST,
		Linear               = GL_LINEAR,
		NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
		NearestMipmapLinear  = GL_NEAREST_MIPMAP_LINEAR,
		LinearMipmapNearest  = GL_LINEAR_MIPMAP_NEAREST,
		LinearMipmapLinear   = GL_LINEAR_MIPMAP_LINEAR
	};

	enum class MagFilterMode
	{
		Nearest = GL_NEAREST,
		Linear  = GL_LINEAR,
	};

	enum class TextureWrappingMode
	{
		Repeat            = GL_REPEAT,
		MirroredRepeat    = GL_MIRRORED_REPEAT,
		ClampToEdge       = GL_CLAMP_TO_EDGE,
		ClampToBorder     = GL_CLAMP_TO_BORDER,
		MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
	};

	enum class InternalImageFormat
	{
		DepthComponent16  = GL_DEPTH_COMPONENT16,
		DepthComponent32  = GL_DEPTH_COMPONENT32,
		DepthComponent32F = GL_DEPTH_COMPONENT32F,
		R8                = GL_R8,
		R16               = GL_R16,
		R32UI             = GL_R32UI,
		R32SI             = GL_R32I,
		R16F              = GL_R16F,
		R32F              = GL_R32F,
		RG8               = GL_RG8,
		RG16              = GL_RG16,
		RG32UI            = GL_RG32UI,
		RG32SI            = GL_RG32I,
		RG16F             = GL_RG16F,
		RG32F             = GL_RG32F,
		RGB8              = GL_RGB8,
		RGB16             = GL_RGB16,
		RGB32UI           = GL_RGB32UI,
		RGB32SI           = GL_RGB32I,
		RGB16F            = GL_RGB16F,
		RGB32F            = GL_RGB32F,
		RGBA8             = GL_RGBA8,
		RGBA16            = GL_RGBA16,
		RGBA32UI          = GL_RGBA32UI,
		RGBA32SI          = GL_RGBA32I,
		RGBA16F           = GL_RGBA16F,
		RGBA32F           = GL_RGBA32F,
	};

	enum class ImageFormat
	{
		DepthComponent = GL_DEPTH_COMPONENT,
		DepthStencil   = GL_DEPTH_STENCIL,
		R              = GL_RED,
		RG             = GL_RG,
		RGB            = GL_RGB,
		RGBA           = GL_RGBA,
		RInteger       = GL_RED_INTEGER,
		RGInteger      = GL_RG_INTEGER,
		RGBInteger     = GL_RGB_INTEGER,
		RGBAInteger    = GL_RGBA_INTEGER,
	};

	class FrameBuffer;

	class Texture
	{
	protected:
		GLuint m_ID;
	public:
		Texture(uint32_t width, uint32_t height);

		virtual ~Texture();

		const uint32_t Width;
		const uint32_t Height;

		[[nodiscard]] GLuint GetID() const { return m_ID; }

		virtual void Bind(uint32_t slot) = 0;

		friend class FrameBuffer;
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D(uint32_t width, uint32_t height, const void* pixelData, InternalImageFormat internalFormat, ImageFormat format, ElementType elementType, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode);
	
		void Bind(uint32_t slot) override;
	};

	class TextureCube final : public Texture
	{
	public:
		TextureCube(uint32_t width, uint32_t height, uint32_t channelCount, const void* facesPixelData, InternalImageFormat internalFormat, ImageFormat format, ElementType elementType, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode);

		void Bind(uint32_t slot) override;
	};
}