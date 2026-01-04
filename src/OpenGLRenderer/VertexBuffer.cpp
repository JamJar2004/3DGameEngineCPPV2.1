#include "VertexBuffer.hpp"

namespace GL
{ 
	void VertexBuffer::Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
	}

	VertexBuffer::VertexBuffer(size_t count, BufferUsage usage, bool isInstanced, const BufferLayout& layout) :
		VertexBuffer(nullptr, count, usage, isInstanced, layout) {}

	VertexBuffer::VertexBuffer(const void* data, size_t count, BufferUsage usage, bool isInstanced, const BufferLayout& layout) :
		m_layout(layout), VertexCount(count), IsInstanced(isInstanced)
	{
		glGenBuffers(1, &m_bufferID);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
		glBufferData(GL_ARRAY_BUFFER, count * layout.GetStride(), data, (GLenum)usage);
	}

	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &m_bufferID);
	}

	void VertexBuffer::SetData(const void* data, size_t offset, size_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	}
}