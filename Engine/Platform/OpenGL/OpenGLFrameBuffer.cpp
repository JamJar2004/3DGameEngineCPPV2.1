#include "OpenGLFrameBuffer.hpp"

#include "OpenGLTexture.hpp"
#include "OpenGLMesh.hpp"

GL::InternalFormat GetInternalFormat(InternalFormat internalFormat)
{
	switch(internalFormat)
	{
		case InternalFormat::DepthComponent16 : return GL::InternalFormat::DepthComponent16 ;
		case InternalFormat::DepthComponent32 : return GL::InternalFormat::DepthComponent32 ;
		case InternalFormat::DepthComponent32F: return GL::InternalFormat::DepthComponent32F;
		case InternalFormat::R8               : return GL::InternalFormat::R8               ;
		case InternalFormat::R16              : return GL::InternalFormat::R16              ;
		case InternalFormat::R32              : return GL::InternalFormat::R32UI            ;
		case InternalFormat::R16F             : return GL::InternalFormat::R16F             ;
		case InternalFormat::R32F             : return GL::InternalFormat::R32F             ;
		case InternalFormat::RG8              : return GL::InternalFormat::RG8              ;
		case InternalFormat::RG16             : return GL::InternalFormat::RG16             ;
		case InternalFormat::RG32             : return GL::InternalFormat::RG32UI           ;
		case InternalFormat::RG16F            : return GL::InternalFormat::RG16F            ;
		case InternalFormat::RG32F            : return GL::InternalFormat::RG32F            ;
		case InternalFormat::RGB8             : return GL::InternalFormat::RGB8             ;
		case InternalFormat::RGB16            : return GL::InternalFormat::RGB16            ;
		case InternalFormat::RGB32            : return GL::InternalFormat::RGB32UI          ;
		case InternalFormat::RGB16F           : return GL::InternalFormat::RGB16F           ;
		case InternalFormat::RGB32F           : return GL::InternalFormat::RGB32F           ;
		case InternalFormat::RGBA8            : return GL::InternalFormat::RGBA8            ;
		case InternalFormat::RGBA16           : return GL::InternalFormat::RGBA16           ;
		case InternalFormat::RGBA32           : return GL::InternalFormat::RGBA32UI         ;
		case InternalFormat::RGBA16F          : return GL::InternalFormat::RGBA16F          ;
		case InternalFormat::RGBA32F          : return GL::InternalFormat::RGBA32F          ;
	}

	return GL::InternalFormat::RGBA8;
}

GL::Format GetFormat(Format format, InternalFormat internalFormat)
{
	bool isInteger =
		internalFormat == InternalFormat::R16 ||
		internalFormat == InternalFormat::R32 ||
		internalFormat == InternalFormat::RG16 ||
		internalFormat == InternalFormat::RG32 ||
		internalFormat == InternalFormat::RGB16 ||
		internalFormat == InternalFormat::RGB32 ||
		internalFormat == InternalFormat::RGBA16 ||
		internalFormat == InternalFormat::RGBA32;

	if(!isInteger)
	{ 
		switch(format)
		{
			case Format::DepthComponent: return GL::Format::DepthComponent;
			case Format::DepthStencil  : return GL::Format::DepthStencil  ;
			case Format::R             : return GL::Format::R             ;
			case Format::RG            : return GL::Format::RG            ;
			case Format::RGB           : return GL::Format::RGB           ;
			case Format::RGBA          : return GL::Format::RGBA          ;
		}
	}
	else
	{
		switch(format)
		{
			case Format::R:    return GL::Format::RInteger;
			case Format::RG:   return GL::Format::RGInteger;
			case Format::RGB:  return GL::Format::RGBInteger;
			case Format::RGBA: return GL::Format::RGBAInteger;
		}
	}
	return GL::Format::RGBA;
}

GL::AttachmentInfo GetAttachment(AttachmentInfo attachmentInfo)
{
	return GL::AttachmentInfo(GetInternalFormat(attachmentInfo.InternalFormat), GetFormat(attachmentInfo.Format, attachmentInfo.InternalFormat), GetElementType(attachmentInfo.ElementType), GetMinFilterMode(attachmentInfo.MinFilter), GetMagFilterMode(attachmentInfo.MagFilter), GetWrappingMode(attachmentInfo.WrappingMode));
}

std::vector<GL::AttachmentInfo> GetAttachments(const std::vector<AttachmentInfo>& attachments)
{
	std::vector<GL::AttachmentInfo> result;
	for(const AttachmentInfo& attachment : attachments)
		result.push_back(GetAttachment(attachment));

	return result;
}
