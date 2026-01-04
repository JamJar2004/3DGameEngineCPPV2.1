#pragma once

#include "../../Rendering/Mesh.hpp"

#include <OpenGLRenderer/VertexArray.hpp>

GL::ElementType GetElementType(TypeInfo* elementType);

class OpenGLMesh final : public Mesh
{
public:
	explicit OpenGLMesh(const Model& model);
protected:
	void Draw(size_t instanceCount) override;
public:
	std::shared_ptr<GL::VertexBuffer> VertexBuffer;
	std::shared_ptr<GL::IndexBuffer>  IndexBuffer;
private:
	GL::VertexArray m_vertexArray;
};

