#pragma once

#include <stdint.h>
#include <GL/glew.h>

#include "Texture.hpp"

namespace GL
{
	class RenderBuffer
	{
	private:
		GLuint m_ID;
	public:
		RenderBuffer(uint32_t width, uint32_t height, InternalImageFormat internalFormat);
		~RenderBuffer();

		void Bind();

		friend class FrameBuffer;
	};
}