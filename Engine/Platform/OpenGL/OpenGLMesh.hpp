#pragma once

#include "../../Rendering/Mesh.hpp"

#include <OpenGLRenderer/VertexArray.hpp>

GL::ElementType GetElementType(ElementType elementType);

class OpenGLMesh : public Mesh
{
public:
	OpenGLMesh(const Model& model);

	std::shared_ptr<GL::VertexBuffer> VertexBuffer;
	std::shared_ptr<GL::IndexBuffer>  IndexBuffer;

	VertexArrayHandle CreateVertexArray(const BufferLayout& instanceLayout) const override;
};

class OpenGLVertexArray : public VertexArray
{
private:
	GL::VertexArray m_vertexArray;

public:
	OpenGLVertexArray(const BufferLayout& instanceLayout, const OpenGLMesh& mesh) : VertexArray(instanceLayout)
	{
		GL::BufferLayout instanceBufferLayout;
		for(const BufferAttribute& attribute : instanceLayout)
		{
			instanceBufferLayout.AddElement(GetElementType(attribute.Type), attribute.Count);
		}

		auto instanceBuffer = std::make_shared<GL::VertexBuffer>(1000, GL::BufferUsage::DynamicCopy, true, instanceBufferLayout);

		m_vertexArray.AddVertexBuffer(mesh.VertexBuffer);
		m_vertexArray.AddVertexBuffer(instanceBuffer);
		m_vertexArray.SetIndexBuffer(mesh.IndexBuffer);
	}

	void Draw(BaseArraySlice instanceElements) override;
};