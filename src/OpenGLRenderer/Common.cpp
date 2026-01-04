#include "Common.hpp"

#include <iostream>
#include <string>

namespace GL
{
	GLenum CheckError()
	{
		GLenum errorCode;
		while((errorCode = glGetError()) != GL_NO_ERROR)
		{
			std::string error;
			switch(errorCode)
			{
				case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
				case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
				case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
				case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
				case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
				case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
				case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			}
			std::cout << error << std::endl;
		}
		return errorCode;
	}

	size_t GetTypeSize(ElementType type)
	{
		switch (type)
		{
			case ElementType::UInt8:
			case ElementType::SInt8:
				return 1;
			case ElementType::UInt16:
			case ElementType::SInt16:
				return 2;
			case ElementType::UInt32:
			case ElementType::SInt32:
			case ElementType::Float32:
				return 4;
			case ElementType::Float64:
				return 8;
		}

		return 0;
	}
}