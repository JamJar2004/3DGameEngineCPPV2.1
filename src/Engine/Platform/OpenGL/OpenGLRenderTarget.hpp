#pragma once

#include <OpenGLRenderer/FrameBuffer.hpp>

#include "../../Rendering/RenderTarget.hpp"

#include "OpenGLTextureAtlas.hpp"

GL::AttachmentInfo GetAttachment(const AttachmentInfo& attachmentInfo);

std::vector<GL::AttachmentInfo> GetAttachments(const std::vector<AttachmentInfo>& attachments);

class OpenGLAttachmentTexture final : public AttachmentTexture
{
private:
	GL::TextureAttachment& m_texture;

public:
	explicit OpenGLAttachmentTexture(GL::TextureAttachment& texture) : AttachmentTexture(texture.Width, texture.Height), m_texture(texture)
	{
	}

	void Bind(uint32_t slot) override
	{
		m_texture.Bind(slot);
	}

	void CopyTo(BitmapBaseHandle cpuBuffer) override
	{
		m_texture.CopyTo(0, 0, cpuBuffer->Width, cpuBuffer->Height, cpuBuffer->GetPixels());
	}
};

class OpenGLDepthAttachmentTexture : public TextureAtlas
{
public:
	explicit OpenGLDepthAttachmentTexture(GL::Texture2D& texture) : TextureAtlas(texture.Width, texture.Height), m_texture(texture)
	{
	}

	void Bind(uint32_t slot) override
	{
		m_texture.Bind(slot);
	}
private:
	GL::Texture2D& m_texture;
};

class OpenGLRenderTarget : public RenderTarget
{
public:
	OpenGLRenderTarget(uint32_t width, uint32_t height) :
		RenderTarget(width, height, 0, false), m_frameBuffer(width, height) {}

	OpenGLRenderTarget(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachmentInfo) :
		RenderTarget(width, height, static_cast<uint32_t>(colorAttachments.size()), depthAttachmentInfo.has_value()),
		m_frameBuffer(width, height, GetAttachments(colorAttachments), depthAttachmentInfo.has_value() ? GetAttachment(depthAttachmentInfo.value()) : std::optional<GL::AttachmentInfo> {})
	{
		m_attachments.resize(colorAttachments.size());
		for(size_t i = 0; i < colorAttachments.size(); i++)
		{
			auto& colorAttachment = m_frameBuffer.GetColorAttachment(i);
			m_attachments[i] = std::make_shared<OpenGLAttachmentTexture>(colorAttachment);
		}

		if(auto depthAttachment = m_frameBuffer.GetDepthAttachment())
		{
			m_depthAttachment = std::make_shared<OpenGLDepthAttachmentTexture>(*depthAttachment);
		}
	}

	  AttachmentHandle GetColorAttachment(size_t index) const override { return m_attachments[index]; }
	TextureAtlasHandle GetDepthAttachment()             const override { return m_depthAttachment;    }

	void Use() override { m_frameBuffer.Bind(GL::FrameBuffer::BindMode::FrameBuffer); }

	void CopyTo(RenderTarget& target, MagFilterMode filterMode, uint32_t bufferType) override
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

		GL::BlitFrameBuffer(m_frameBuffer, static_cast<OpenGLRenderTarget&>(target).m_frameBuffer, GetMagFilterMode(filterMode), mask);
	}

	void Clear(int bufferType) override;

	void ClearColorBuffer(PixelFormat format, const void* value) override;

private:
	GL::FrameBuffer m_frameBuffer;
	std::vector<AttachmentHandle> m_attachments;
	TextureAtlasHandle m_depthAttachment;
};