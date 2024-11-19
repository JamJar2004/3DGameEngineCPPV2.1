#include "OpenGLMesh.hpp"

#include "OpenGLCommon.hpp"




OpenGLMesh::OpenGLMesh(const Model& model) : Mesh(model)
{
    GL::BufferLayout vertexBufferLayout;
    for(const BufferAttribute& attribute : model.Vertices->GetLayout())
    {
        vertexBufferLayout.AddElement(GetElementType(attribute.Type), attribute.Count);
    }

    VertexBuffer = std::make_shared<GL::VertexBuffer>(model.Vertices->Data(), model.Vertices->Count(), GL::BufferUsage::StaticDraw, false, vertexBufferLayout);
    IndexBuffer  = std::make_shared<GL::IndexBuffer>(*model.Indices, GL::BufferUsage::StaticDraw);

	/*GL::BufferLayout vertexBufferLayout;
	for(const BufferAttribute& attribute : model.Vertices->GetLayout())
		vertexBufferLayout.AddElement(GetElementType(attribute.Type), attribute.Count);

	auto vertexBuffer = std::make_shared<GL::VertexBuffer>(model.Vertices->Data(), model.Vertices->Count(), GL::BufferUsage::StaticDraw, false, vertexBufferLayout);
	auto indexBuffer = std::make_shared<GL::IndexBuffer>(*model.Indices, GL::BufferUsage::StaticDraw);

	GL::BufferLayout instanceBufferLayout;
    for(const BufferAttribute& attribute : instanceLayout)
        instanceBufferLayout.AddElement(GetElementType(attribute.Type), attribute.Count);

	auto instanceBuffer = std::make_shared<GL::VertexBuffer>(MAX_INSTANCE_COUNT, GL::BufferUsage::DynamicCopy, true, instanceBufferLayout);
	
	m_vertexArray.AddVertexBuffer(vertexBuffer);
	m_vertexArray.AddVertexBuffer(instanceBuffer);
	m_vertexArray.SetIndexBuffer(indexBuffer);*/
}

VertexArrayHandle OpenGLMesh::CreateVertexArray(const BufferLayout& instanceLayout) const
{
    return std::make_shared<OpenGLVertexArray>(instanceLayout, *this);
}

void OpenGLVertexArray::Draw(BaseArraySlice instanceElements)
{
    m_vertexArray.Bind();

    size_t instanceCount = 1;
    if(instanceElements)
    {
        size_t elementSize = instanceElements.GetLayout().GetStride();
        instanceCount = instanceElements.Count();
        m_vertexArray[1]->SetData(instanceElements.Data(), 0, instanceElements.Count() * elementSize);
    }
    m_vertexArray.Draw(GL::DrawMode::Triangles, instanceCount);
}