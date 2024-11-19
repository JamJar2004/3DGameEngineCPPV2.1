#pragma once

#include <OpenGLRenderer/FrameBuffer.hpp>

#include "../../Rendering/FrameBuffer.hpp"
#include "OpenGLTexture.hpp"

GL::AttachmentInfo GetAttachment(AttachmentInfo attachmentInfo);

std::vector<GL::AttachmentInfo> GetAttachments(const std::vector<AttachmentInfo>& attachments);

class OpenGLAttachmentTexture : public AttachmentTexture
{
private:
	GL::TextureAttachment& m_texture;

public:
	OpenGLAttachmentTexture(GL::TextureAttachment& texture) : AttachmentTexture(texture.Width, texture.Height), m_texture(texture)
	{
	}

	virtual void Bind(uint32_t slot) override
	{
		m_texture.Bind(slot);
	}

	virtual void CopyTo(BaseBitmapHandle cpuBuffer) override
	{
		m_texture.CopyTo(0, 0, cpuBuffer->Width, cpuBuffer->Height, cpuBuffer->GetPixels());
	}
};

class OpenGLDepthAttachmentTexture : public Texture
{
private:
	GL::Texture2D& m_texture;

public:
	OpenGLDepthAttachmentTexture(GL::Texture2D& texture) : Texture(texture.Width, texture.Height), m_texture(texture)
	{
	}

	virtual void Bind(uint32_t slot) override
	{
		m_texture.Bind(slot);
	}
};

class OpenGLFrameBuffer : public FrameBuffer
{
private:
	GL::FrameBuffer m_frameBuffer;
	std::vector<AttachmentHandle> m_attachments;
	TextureHandle m_depthAttachment;
public:
	OpenGLFrameBuffer(uint32_t width, uint32_t height) : 
		FrameBuffer(width, height, 0, false), m_frameBuffer(width, height) {}

	OpenGLFrameBuffer(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachmentInfo) :
		FrameBuffer(width, height, uint32_t(colorAttachments.size()), depthAttachmentInfo.has_value()),
		m_frameBuffer(width, height, GetAttachments(colorAttachments), depthAttachmentInfo.has_value() ? GetAttachment(depthAttachmentInfo.value()) : std::optional<GL::AttachmentInfo> {})
	{
		m_attachments.resize(colorAttachments.size());
		for(size_t i = 0; i < colorAttachments.size(); i++)
		{
			auto& colorAttachment = m_frameBuffer.GetColorAttachment(i);
			m_attachments[i] = std::make_shared<OpenGLAttachmentTexture>(colorAttachment);
		}

		auto depthAttachment = m_frameBuffer.GetDepthAttachment();
		if(depthAttachment)
			m_depthAttachment = std::make_shared<OpenGLDepthAttachmentTexture>(*depthAttachment);
	}

	virtual AttachmentHandle GetColorAttachment(size_t index) const override { return m_attachments[index]; }
	virtual TextureHandle    GetDepthAttachment()             const override { return m_depthAttachment;    }

	virtual void Bind() override { m_frameBuffer.Bind(GL::FrameBuffer::BindMode::FrameBuffer); }

	virtual void CopyTo(FrameBufferHandle dest, MagFilterMode filterMode, uint32_t bufferType) override
	{
		uint32_t mask = 0;

		if(bufferType & ScreenBufferType::ColorBuffer)
			mask |= GL::BufferTarget::ColorBuffer;

		if(bufferType & ScreenBufferType::DepthBuffer)
			mask |= GL::BufferTarget::DepthBuffer;

		if(bufferType & ScreenBufferType::StencilBuffer)
			mask |= GL::BufferTarget::StencilBuffer;

		GL::BlitFrameBuffer(m_frameBuffer, std::dynamic_pointer_cast<OpenGLFrameBuffer>(dest)->m_frameBuffer, GetMagFilterMode(filterMode), mask);
	}
};