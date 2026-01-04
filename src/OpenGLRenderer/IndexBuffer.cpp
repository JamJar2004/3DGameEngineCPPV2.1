#include "IndexBuffer.hpp"

namespace GL
{
	void IndexBuffer::Bind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
	}

	IndexBuffer::IndexBuffer(const std::vector<uint32_t>& indices, BufferUsage usage) : IndexCount(indices.size())
	{
		glGenBuffers(1, &m_bufferID);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
		glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), (GLenum)usage);
	}

	IndexBuffer::~IndexBuffer()
	{
		glDeleteBuffers(1, &m_bufferID);
	}

	void IndexBuffer::SetData(const std::vector<uint32_t>& indices, size_t offset)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, indices.size(), indices.data());
	}
}
