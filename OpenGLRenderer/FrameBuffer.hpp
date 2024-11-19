#pragma once

#include <stdint.h>
#include <memory>
#include <vector>
#include <optional>
#include <GL/glew.h>

#include "Texture.hpp"
#include "RenderBuffer.hpp"

namespace GL
{
	enum BufferTarget
	{
		ColorBuffer   = GL_COLOR_BUFFER_BIT,
		DepthBuffer   = GL_DEPTH_BUFFER_BIT,
		StencilBuffer = GL_STENCIL_BUFFER_BIT,
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

	class TextureAttachment : public Texture2D
	{
	public:
		const size_t Index;
		const AttachmentInfo Attachment;

		TextureAttachment(uint32_t width, uint32_t height, size_t index, InternalFormat internalFormat, Format format, ElementType elementType, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode);

		void CopyTo(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height, void* cpuBuffer);
	};

	class FrameBuffer
	{
	private:
		GLuint m_ID;

		std::vector<TextureAttachment> m_colorAttachments;

		std::shared_ptr<Texture2D> m_depthAttachment;

		std::shared_ptr<RenderBuffer> m_renderBuffer;
	public:
		enum class BindMode
		{
			FrameBuffer     = GL_FRAMEBUFFER,
			ReadFrameBuffer = GL_READ_FRAMEBUFFER,
			DrawFrameBuffer = GL_DRAW_FRAMEBUFFER,
		};

		FrameBuffer(uint32_t width, uint32_t height) : m_ID(0), Width(width), Height(height) {}

		FrameBuffer(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachment);

		~FrameBuffer();

		const uint32_t Width;
		const uint32_t Height;

		                TextureAttachment& GetColorAttachment(size_t index) { return m_colorAttachments[index]; }
		std::shared_ptr<Texture2D>         GetDepthAttachment()             { return m_depthAttachment;         }

		std::shared_ptr<RenderBuffer> GetRenderBuffer() const { return m_renderBuffer; }

		void Bind(BindMode mode);
	};

	void BlitFrameBuffer(FrameBuffer& source, FrameBuffer& dest, MagFilterMode filterMode, uint32_t targets);
}