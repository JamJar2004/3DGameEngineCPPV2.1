#include "FrameBuffer.hpp"

#include <iostream>

namespace GL
{
	TextureAttachment::TextureAttachment(uint32_t width, uint32_t height, size_t index, InternalFormat internalFormat, Format format, ElementType elementType, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode) : 
		Texture2D(width, height, nullptr, internalFormat, format, elementType, minFilter, magFilter, wrappingMode), 
		Index(index), Attachment(internalFormat, format, elementType, minFilter, magFilter, wrappingMode)
	{
	}

	void TextureAttachment::CopyTo(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height, void* cpuBuffer)
	{
		glReadBuffer(GLenum(GL_COLOR_ATTACHMENT0 + Index));
		glReadPixels(xOffset, yOffset, width, height, GLenum(Attachment.Format), GLenum(Attachment.ElementType), cpuBuffer);
	}

	FrameBuffer::FrameBuffer(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachment) : Width(width), Height(height)
	{
		glGenFramebuffers(1, &m_ID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);

		std::vector<GLenum> attachments;
		m_colorAttachments.reserve(colorAttachments.size());
		attachments.resize(colorAttachments.size());
		for(size_t i = 0; i < colorAttachments.size(); i++)
		{
			attachments[i] = GLenum(GL_COLOR_ATTACHMENT0 + i);
			const AttachmentInfo& attachmentInfo = colorAttachments[i];
			auto& attachment = m_colorAttachments.emplace_back(width, height, i, attachmentInfo.InternalFormat, attachmentInfo.Format, attachmentInfo.ElementType, attachmentInfo.MinFilter, attachmentInfo.MagFilter, attachmentInfo.WrappingMode);
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i], GL_TEXTURE_2D, attachment.m_ID, 0);
		}

		if(colorAttachments.size() > 0)
			glDrawBuffers(GLsizei(colorAttachments.size()), attachments.data());

		if(depthAttachment.has_value())
		{
			m_depthAttachment = std::make_shared<Texture2D>(width, height, nullptr, depthAttachment.value().InternalFormat, depthAttachment.value().Format, depthAttachment.value().ElementType, depthAttachment.value().MinFilter, depthAttachment.value().MagFilter, depthAttachment.value().WrappingMode);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment->m_ID, 0);
		}
		else
		{
			m_renderBuffer = std::make_shared<RenderBuffer>(width, height, InternalFormat::DepthComponent16);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer->m_ID);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer is not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &m_ID);
	}

	void FrameBuffer::Bind(BindMode mode)
	{
		glBindFramebuffer((GLenum)mode, m_ID);

		if(m_renderBuffer)
			m_renderBuffer->Bind();

		glViewport(0, 0, Width, Height);
	}

	void BlitFrameBuffer(FrameBuffer& source, FrameBuffer& dest, MagFilterMode filterMode, uint32_t targets)
	{
		source.Bind(FrameBuffer::BindMode::ReadFrameBuffer);
		dest.Bind(FrameBuffer::BindMode::DrawFrameBuffer);

		glBlitFramebuffer(0, 0, source.Width, source.Height, 0, 0, dest.Width, dest.Height, (GLbitfield)targets, (GLenum)filterMode);
	}
}