#include "Renderer.hpp"

#include <iostream>

namespace GL
{

	void Enable (EnableFlags flags) { glEnable((GLenum)flags);  }
	void Disable(EnableFlags flags) { glDisable((GLenum)flags); }

	void Clear(uint32_t targets) 
	{
		glClear((GLbitfield)targets);
	}

	void SetClearColor(const glm::vec4& color) { glClearColor(color.r, color.g, color.b, color.a); }

	void FrontFace(FrontFaceMode mode) { glFrontFace((GLenum)mode); }

	void CullFace(CullFaceMode mode) { glCullFace((GLenum)mode); }

	void DepthMask(bool write)
	{
		if(write)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}

	void BlendFunc(BlendFactor sourceFactor, BlendFactor destFactor) { glBlendFunc((GLenum)sourceFactor, (GLenum)destFactor); }

	void DepthFunc(DepthFunction function) { glDepthFunc((GLenum)function); }	
}
