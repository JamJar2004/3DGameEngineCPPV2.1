#pragma once

#include <memory>

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

namespace GL
{
	enum class DrawMode
	{
		Triangles     = GL_TRIANGLES,
		TriangleStrip = GL_TRIANGLE_STRIP,
		TriangleFan   = GL_TRIANGLE_FAN,
	};

	class VertexArray
	{
	private:
		GLuint m_ID;

		std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;

		std::shared_ptr<IndexBuffer> m_indexBuffer;

		size_t m_attributeIndex;

		void AddAttribute(size_t count, ElementType type, size_t stride, size_t offset, bool isInstanced);
	public:
		VertexArray();

		~VertexArray();

		void Bind() const { glBindVertexArray(m_ID); }

		std::shared_ptr<VertexBuffer> operator[](size_t index) const { return m_vertexBuffers[index]; }

		std::shared_ptr<IndexBuffer> GetIndexBuffer() { return m_indexBuffer; }

		void AddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer);

		void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer);

		void Draw(size_t vertexBufferIndex, DrawMode mode, size_t instanceCount) const;
		void Draw(DrawMode mode, size_t instanceCount) const;
	};
}