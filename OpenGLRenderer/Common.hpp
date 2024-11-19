#pragma once

#define GLEW_STATIC

#include <GL/glew.h>

namespace GL
{ 
	GLenum CheckError();

	enum class ElementType
	{
		UInt8   = GL_UNSIGNED_BYTE,
		SInt8   = GL_BYTE,
		UInt16  = GL_UNSIGNED_SHORT,
		SInt16  = GL_SHORT,
		UInt32  = GL_UNSIGNED_INT,
		SInt32  = GL_INT,
		Float32 = GL_FLOAT,
		Float64 = GL_DOUBLE,
	};

	size_t GetTypeSize(ElementType type);

	enum class BufferUsage
	{
		StaticDraw  = GL_STATIC_DRAW,
		DynamicDraw = GL_DYNAMIC_DRAW,
		StreamDraw  = GL_STREAM_DRAW,
		StaticCopy  = GL_STATIC_COPY,
		DynamicCopy = GL_DYNAMIC_COPY,
		StreamCopy  = GL_STREAM_COPY,
	};
}