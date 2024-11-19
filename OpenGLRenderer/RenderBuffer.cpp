#include "RenderBuffer.hpp"

namespace GL
{
	RenderBuffer::RenderBuffer(uint32_t width, uint32_t height, InternalFormat internalFormat)
	{
		glGenRenderbuffers(1, &m_ID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_ID);
		glRenderbufferStorage(GL_RENDERBUFFER, (GLenum)internalFormat, width, height);
	}

	RenderBuffer::~RenderBuffer()
	{
		glDeleteRenderbuffers(1, &m_ID);
	}

	void RenderBuffer::Bind()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, m_ID);
	}
}
