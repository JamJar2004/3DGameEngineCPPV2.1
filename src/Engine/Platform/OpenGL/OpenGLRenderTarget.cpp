#include "OpenGLRenderTarget.hpp"

#include "OpenGLTextureAtlas.hpp"
#include "OpenGLMesh.hpp"
#include "OpenGLRenderer/Renderer.hpp"

GL::InternalImageFormat GetFrameBufferInternalFormat(InternalImageFormat internalFormat)
{
	switch(internalFormat)
	{
		case InternalImageFormat::DepthComponent16 : return GL::InternalImageFormat::DepthComponent16 ;
		case InternalImageFormat::DepthComponent32 : return GL::InternalImageFormat::DepthComponent32 ;
		case InternalImageFormat::DepthComponent32F: return GL::InternalImageFormat::DepthComponent32F;
		case InternalImageFormat::R8               : return GL::InternalImageFormat::R8               ;
		case InternalImageFormat::R16              : return GL::InternalImageFormat::R16              ;
		case InternalImageFormat::R32              : return GL::InternalImageFormat::R32UI            ;
		case InternalImageFormat::R16F             : return GL::InternalImageFormat::R16F             ;
		case InternalImageFormat::R32F             : return GL::InternalImageFormat::R32F             ;
		case InternalImageFormat::RG8              : return GL::InternalImageFormat::RG8              ;
		case InternalImageFormat::RG16             : return GL::InternalImageFormat::RG16             ;
		case InternalImageFormat::RG32             : return GL::InternalImageFormat::RG32UI           ;
		case InternalImageFormat::RG16F            : return GL::InternalImageFormat::RG16F            ;
		case InternalImageFormat::RG32F            : return GL::InternalImageFormat::RG32F            ;
		case InternalImageFormat::RGB8             : return GL::InternalImageFormat::RGB8             ;
		case InternalImageFormat::RGB16            : return GL::InternalImageFormat::RGB16            ;
		case InternalImageFormat::RGB32            : return GL::InternalImageFormat::RGB32UI          ;
		case InternalImageFormat::RGB16F           : return GL::InternalImageFormat::RGB16F           ;
		case InternalImageFormat::RGB32F           : return GL::InternalImageFormat::RGB32F           ;
		case InternalImageFormat::RGBA8            : return GL::InternalImageFormat::RGBA8            ;
		case InternalImageFormat::RGBA16           : return GL::InternalImageFormat::RGBA16           ;
		case InternalImageFormat::RGBA32           : return GL::InternalImageFormat::RGBA32UI         ;
		case InternalImageFormat::RGBA16F          : return GL::InternalImageFormat::RGBA16F          ;
		case InternalImageFormat::RGBA32F          : return GL::InternalImageFormat::RGBA32F          ;
	}

	return GL::InternalImageFormat::RGBA8;
}

GL::ImageFormat GetFrameBufferFormat(ImageFormat format, InternalImageFormat internalFormat)
{
	bool isInteger =
		internalFormat == InternalImageFormat::R16 ||
		internalFormat == InternalImageFormat::R32 ||
		internalFormat == InternalImageFormat::RG16 ||
		internalFormat == InternalImageFormat::RG32 ||
		internalFormat == InternalImageFormat::RGB16 ||
		internalFormat == InternalImageFormat::RGB32 ||
		internalFormat == InternalImageFormat::RGBA16 ||
		internalFormat == InternalImageFormat::RGBA32;

	if(!isInteger)
	{
		switch(format)
		{
			case ImageFormat::DepthComponent: return GL::ImageFormat::DepthComponent;
			case ImageFormat::DepthStencil  : return GL::ImageFormat::DepthStencil  ;
			case ImageFormat::R             : return GL::ImageFormat::R             ;
			case ImageFormat::RG            : return GL::ImageFormat::RG            ;
			case ImageFormat::RGB           : return GL::ImageFormat::RGB           ;
			case ImageFormat::RGBA          : return GL::ImageFormat::RGBA          ;
		}
	}
	else
	{
		switch(format)
		{
			case ImageFormat::R:    return GL::ImageFormat::RInteger;
			case ImageFormat::RG:   return GL::ImageFormat::RGInteger;
			case ImageFormat::RGB:  return GL::ImageFormat::RGBInteger;
			case ImageFormat::RGBA: return GL::ImageFormat::RGBAInteger;
			default: ;
		}
	}
	return GL::ImageFormat::RGBA;
}

GL::AttachmentInfo GetAttachment(const AttachmentInfo& attachmentInfo)
{
	return GL::AttachmentInfo(GetFrameBufferInternalFormat(attachmentInfo.InternalFormat), GetFrameBufferFormat(attachmentInfo.Format, attachmentInfo.InternalFormat), GetElementType(attachmentInfo.PixelElementType), GetMinFilterMode(attachmentInfo.MinFilter), GetMagFilterMode(attachmentInfo.MagFilter), GetWrappingMode(attachmentInfo.WrappingMode));
}

std::vector<GL::AttachmentInfo> GetAttachments(const std::vector<AttachmentInfo>& attachments)
{
	std::vector<GL::AttachmentInfo> result;
	for(const AttachmentInfo& attachment : attachments)
	{
		result.push_back(GetAttachment(attachment));
	}
	return result;
}

void OpenGLRenderTarget::Clear(const int bufferType)
{
	uint32_t mask = 0;

	if(bufferType & ColorBuffer)
	{
	 	mask |= GL::BufferTarget::ColorBuffer;
	}

	if(bufferType & DepthBuffer)
	{
	 	mask |= GL::BufferTarget::DepthBuffer;
	}

	if(bufferType & StencilBuffer)
	{
	 	mask |= GL::BufferTarget::StencilBuffer;
	}

	m_frameBuffer.Bind(GL::FrameBuffer::BindMode::FrameBuffer);
	GL::Clear(mask);
}

template<typename TElement, size_t Dimensions>
static void ClearBuffer(const void* value)
{
	GL::ClearFloatBuffer<Dimensions>(*static_cast<const glm::vec<Dimensions, TElement>*>(value));
}

void OpenGLRenderTarget::ClearColorBuffer(PixelFormat format, const void* value)
{
	m_frameBuffer.Bind(GL::FrameBuffer::BindMode::FrameBuffer);
	if (format.BufferElementType == TypeInfo::Get<glm::float32>())
	{
		switch (format.ChannelCount)
		{
			case 1:
				ClearBuffer<glm::float32, 1>(value);
			case 2:
				ClearBuffer<glm::float32, 2>(value);
			case 3:
				ClearBuffer<glm::float32, 3>(value);
			case 4:
				ClearBuffer<glm::float32, 4>(value);
			default: ;
		}
	}
	else if (format.BufferElementType == TypeInfo::Get<glm::uint32>())
	{
		switch (format.ChannelCount)
		{
			case 1:
				ClearBuffer<glm::uint32, 1>(value);
			case 2:
				ClearBuffer<glm::uint32, 2>(value);
			case 3:
				ClearBuffer<glm::uint32, 3>(value);
			case 4:
				ClearBuffer<glm::uint32, 4>(value);
			default: ;
		}
	}
	else if (format.BufferElementType == TypeInfo::Get<glm::int32>())
	{
		switch (format.ChannelCount)
		{
			case 1:
				ClearBuffer<glm::int32, 1>(value);
			case 2:
				ClearBuffer<glm::int32, 2>(value);
			case 3:
				ClearBuffer<glm::int32, 3>(value);
			case 4:
				ClearBuffer<glm::int32, 4>(value);
			default: ;
		}
	}
}
