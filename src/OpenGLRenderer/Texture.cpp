#include "Texture.hpp"

namespace GL
{
	//GLenum GetElementType(InternalFormat internalFormat)
	//{
	//	switch(internalFormat)
	//	{
	//		case InternalFormat::R8:
	//		case InternalFormat::RG8:
	//		case InternalFormat::RGB8:
	//		case InternalFormat::RGBA8:
	//			return GL_UNSIGNED_BYTE;
	//		case InternalFormat::R16:
	//		case InternalFormat::RG16:
	//		case InternalFormat::RGB16:
	//		case InternalFormat::RGBA16:
	//		case InternalFormat::DepthComponent16:
	//			return GL_UNSIGNED_SHORT;
	//		case InternalFormat::DepthComponent32:
	//			return GL_UNSIGNED_INT;
	//		case InternalFormat::R16F:
	//		case InternalFormat::R32F:
	//		case InternalFormat::RG16F:
	//		case InternalFormat::RG32F:
	//		case InternalFormat::RGB16F:
	//		case InternalFormat::RGB32F:
	//		case InternalFormat::RGBA16F:
	//		case InternalFormat::RGBA32F:
	//		case InternalFormat::DepthComponent32F:
	//			return GL_FLOAT;
	//	}
	//}

	Texture::Texture(uint32_t width, uint32_t height) : Width(width), Height(height)
	{
		glGenTextures(1, &m_ID);
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &m_ID);
	}

	Texture2D::Texture2D(uint32_t width, uint32_t height, const void* pixelData, InternalImageFormat internalFormat, ImageFormat format, ElementType elementType, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) :
		Texture(width, height)
	{
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, (GLint)internalFormat, width, height, 0, (GLenum)format, (GLenum)elementType, pixelData);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)magFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)wrappingMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)wrappingMode);

		if( minFilter == MinFilterMode::NearestMipmapNearest ||
			minFilter == MinFilterMode::NearestMipmapLinear  ||
			minFilter == MinFilterMode::LinearMipmapNearest  ||
			minFilter == MinFilterMode::LinearMipmapLinear)
			glGenerateMipmap(GL_TEXTURE_2D);
	}

	void Texture2D::Bind(uint32_t slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_ID);
	}

	TextureCube::TextureCube(uint32_t width, uint32_t height, uint32_t channelCount, const void* facesPixelData, InternalImageFormat internalFormat, ImageFormat format, ElementType elementType, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) :
		Texture(width, height)
	{
		size_t faceSize = size_t(width) * height * channelCount;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
		
		std::array<GLenum, 6> faceTargets = 
		{
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};

		for(size_t i = 0; i < faceTargets.size(); i++)
		{
			glTexImage2D(faceTargets[i], 0, (GLint)internalFormat, width, height, 0, (GLenum)format, (GLenum)elementType, (uint8_t*)facesPixelData + faceSize * i);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, (GLint)minFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, (GLint)magFilter);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, (GLint)wrappingMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, (GLint)wrappingMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, (GLint)wrappingMode);

		if(minFilter == MinFilterMode::NearestMipmapNearest ||
		   minFilter == MinFilterMode::NearestMipmapLinear ||
		   minFilter == MinFilterMode::LinearMipmapNearest ||
		   minFilter == MinFilterMode::LinearMipmapLinear)
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	void TextureCube::Bind(uint32_t slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
	}
}