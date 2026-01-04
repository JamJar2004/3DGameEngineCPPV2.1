#pragma once

#include <vector>

#include "Common.hpp"

namespace GL
{
	class IndexBuffer
	{
	private:
		GLuint m_bufferID;

		void Bind();
	public:
		IndexBuffer(const std::vector<uint32_t>& indices, BufferUsage usage);

		~IndexBuffer();

		const size_t IndexCount;

		void SetData(const std::vector<uint32_t>& indices, size_t offset);

		friend class VertexArray;
	};
}