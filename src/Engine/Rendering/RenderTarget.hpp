#pragma once

#include "TextureAtlas.hpp"
#include "Mesh.hpp"

enum ScreenBufferType
{
	ColorBuffer   = 1,
	DepthBuffer   = 2,
	StencilBuffer = 4,
};

struct AttachmentInfo
{
	AttachmentInfo(InternalImageFormat internalFormat, ImageFormat format, TypeInfo* pixelElementType, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) :
		InternalFormat(internalFormat),
		Format(format),
		PixelElementType(pixelElementType),
		MinFilter(minFilter),
		MagFilter(magFilter),
		WrappingMode(wrappingMode) {}

	InternalImageFormat InternalFormat;
	ImageFormat         Format;
	TypeInfo*           PixelElementType;
	MinFilterMode       MinFilter;
	MagFilterMode       MagFilter;
	TextureWrappingMode WrappingMode;
};

class RenderTarget;

using RenderTargetHandle = std::shared_ptr<RenderTarget>;

class AttachmentTexture : public TextureAtlas
{
public:
	AttachmentTexture(uint32_t width, uint32_t height) : TextureAtlas(width, height) {}

	virtual void CopyTo(BitmapBaseHandle cpuBuffer) = 0;
};

using AttachmentHandle = std::shared_ptr<AttachmentTexture>;

class RenderTarget
{
public:
	RenderTarget(uint32_t width, uint32_t height, uint32_t colorAttachmentCount, bool hasDepthAttachment) :
		Width(width), Height(height),
		ColorAttachmentCount(colorAttachmentCount), HasDepthAttachment(hasDepthAttachment),
		ClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) {}

	virtual ~RenderTarget() = default;

	[[nodiscard]] virtual   AttachmentHandle GetColorAttachment(size_t index) const = 0;
	[[nodiscard]] virtual TextureAtlasHandle GetDepthAttachment()             const = 0;

	union
	{
		struct
		{
			const uint32_t Width;
			const uint32_t Height;
		};
		const glm::uvec2 Size {};
	};

	const size_t ColorAttachmentCount;

	const bool HasDepthAttachment;

	glm::vec4 ClearColor;

	virtual void Clear(int bufferType) = 0;

	virtual void ClearColorBuffer(PixelFormat format, const void* value) = 0;

	virtual void CopyTo(RenderTarget& target, MagFilterMode filterMode, uint32_t bufferType) = 0;
protected:
	virtual void Use() = 0;

	template<ShallowCopyable TElement>
	friend class RenderStream;
};