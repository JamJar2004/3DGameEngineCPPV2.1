#pragma once

#include <stdint.h>

#include "Texture.hpp"
#include "Mesh.hpp"

enum ScreenBufferType
{
	ColorBuffer   = 1,
	DepthBuffer   = 2,
	StencilBuffer = 4,
};

struct AttachmentInfo
{
	AttachmentInfo(InternalFormat internalFormat, Format format, ElementType elementType, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode) :
		InternalFormat(internalFormat), Format(format), ElementType(elementType), MinFilter(minFilter), MagFilter(magFilter), WrappingMode(wrappingMode) {}

	InternalFormat InternalFormat;
	Format         Format;
	ElementType    ElementType;
	MinFilterMode  MinFilter;
	MagFilterMode  MagFilter;
	WrappingMode   WrappingMode;
};

class FrameBuffer;

using FrameBufferHandle = std::shared_ptr<FrameBuffer>;

class AttachmentTexture : public Texture
{
public:
	AttachmentTexture(uint32_t width, uint32_t height) : Texture(width, height) {}

	virtual void CopyTo(BaseBitmapHandle cpuBuffer) = 0;
};

using AttachmentHandle = std::shared_ptr<AttachmentTexture>;

class FrameBuffer
{
public:
	FrameBuffer(uint32_t width, uint32_t height, uint32_t colorAttachmentCount, bool hasDepthAttachment) :
		Width(width), Height(height),
		ColorAttachmentCount(colorAttachmentCount), HasDepthAttachment(hasDepthAttachment) {}

	virtual AttachmentHandle GetColorAttachment(size_t index) const = 0;
	virtual TextureHandle    GetDepthAttachment()             const = 0;

	union
	{
		struct
		{
			const uint32_t Width;
			const uint32_t Height;
		};
		const glm::uvec2 Size;
	};

	const size_t ColorAttachmentCount;

	const bool HasDepthAttachment;

	virtual void Bind() = 0;

	virtual void CopyTo(FrameBufferHandle dest, MagFilterMode filterMode, uint32_t bufferType) = 0;
};