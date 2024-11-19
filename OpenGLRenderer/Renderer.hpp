#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "FrameBuffer.hpp"

namespace GL
{
	enum class EnableFlags
	{
		Blend       = GL_BLEND,
		ClipPlane0  = GL_CLIP_PLANE0,
		ClipPlane1  = GL_CLIP_PLANE1,
		ClipPlane2  = GL_CLIP_PLANE2,
		ClipPlane3  = GL_CLIP_PLANE3,
		ClipPlane4  = GL_CLIP_PLANE4,
		ClipPlane5  = GL_CLIP_PLANE5,
		CullFace    = GL_CULL_FACE,
		DepthTest   = GL_DEPTH_TEST,
		DepthClamp  = GL_DEPTH_CLAMP,
		MultiSample = GL_MULTISAMPLE,
	};

	enum class CullFaceMode
	{
		Back  = GL_BACK,
		Front = GL_FRONT,
	};

	enum class FrontFaceMode
	{
		Clockwise     = GL_CW,
		AntiClockwise = GL_CCW,
	};

	enum class BlendFactor
	{
		Zero                = GL_ZERO,
		One                 = GL_ONE,
		SourceColor         = GL_SRC_COLOR,
		DestColor           = GL_DST_COLOR,
		OneMinusSourceColor = GL_ONE_MINUS_SRC_COLOR,
		OneMinusDestColor   = GL_ONE_MINUS_SRC_COLOR,
		SourceAlpha         = GL_SRC_ALPHA,
		DestAlpha           = GL_DST_ALPHA,
		OneMinusSourceAlpha = GL_ONE_MINUS_SRC_ALPHA,
		OneMinusDestAlpha   = GL_ONE_MINUS_DST_ALPHA,
	};
	
	enum class DepthFunction
	{
		Never          = GL_NEVER,
		Equal          = GL_EQUAL,
		NotEqual       = GL_NOTEQUAL,
		Less           = GL_LESS,
		Greater        = GL_GREATER,
		LessOrEqual    = GL_LEQUAL,
		GreaterOrEqual = GL_GEQUAL,
		Always         = GL_ALWAYS
	};

	void Enable(EnableFlags flags);
	void Disable(EnableFlags flags);

	void Clear(uint32_t targets);

	template<size_t D>
	void ClearFloatBuffer(const glm::vec<D, glm::highp_float32, glm::defaultp>& value)
	{
		glClearBufferfv(GL_COLOR, 0, &value[0]);
	}

	template<size_t D>
	void ClearSIntBuffer(const glm::vec<D, glm::int32, glm::defaultp>& value)
	{
		glClearBufferiv(GL_COLOR, 0, &value[0]);
	}

	template<size_t D>
	void ClearUIntBuffer(const glm::vec<D, glm::uint, glm::defaultp>& value)
	{
		glClearBufferuiv(GL_COLOR, 0, &value[0]);
	}

	void SetClearColor(const glm::vec4& color);

	void FrontFace(FrontFaceMode mode);
	void CullFace(CullFaceMode mode);

	void DepthMask(bool write);

	void BlendFunc(BlendFactor sourceFactor, BlendFactor destFactor);
	void DepthFunc(DepthFunction function);
}