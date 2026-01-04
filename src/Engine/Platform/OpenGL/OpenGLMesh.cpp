#include "OpenGLMesh.hpp"

#include "OpenGLCommon.hpp"

OpenGLMesh::OpenGLMesh(const Model& model) : Mesh(model)
{
    GL::BufferLayout vertexBufferLayout;
    for(const BufferAttribute& attribute : model.GetLayout())
    {
        vertexBufferLayout.AddElement(GetElementType(attribute.ElementType), attribute.Count);
    }

    const auto vertexBuffer = std::make_shared<GL::VertexBuffer>(model.Vertices->Data(), model.Vertices->Count(), GL::BufferUsage::StaticDraw, false, vertexBufferLayout);
    const auto indexBuffer  = std::make_shared<GL::IndexBuffer>(*model.Indices, GL::BufferUsage::StaticDraw);

	m_vertexArray.AddVertexBuffer(vertexBuffer);
	m_vertexArray.SetIndexBuffer(indexBuffer);
}

void OpenGLMesh::Draw(size_t instanceCount)
{
    m_vertexArray.Bind();
    m_vertexArray.Draw(GL::DrawMode::Triangles, instanceCount);
}
