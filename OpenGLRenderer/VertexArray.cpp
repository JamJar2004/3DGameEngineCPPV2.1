#include "VertexArray.hpp"
#include <iostream>

namespace GL
{ 
    VertexArray::VertexArray() : m_attributeIndex(0)
    {
        glGenVertexArrays(1, &m_ID);
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &m_ID);
    }

    void VertexArray::AddAttribute(size_t count, ElementType type, size_t stride, size_t offset, bool isInstanced)
    {
        glEnableVertexAttribArray(GLuint(m_attributeIndex));

        if(type == ElementType::Float32 || type == ElementType::Float64)
        {
            glVertexAttribPointer(GLuint(m_attributeIndex), GLint(count), (GLenum)type, GL_FALSE, GLsizei(stride), (void*)offset);
        }
        else
        {
            glVertexAttribIPointer(GLuint(m_attributeIndex), GLint(count), (GLenum)type, GLsizei(stride), (void*)offset);
        }

        if(isInstanced)
        {
            glVertexAttribDivisor(GLuint(m_attributeIndex), 1);
        }
        ++m_attributeIndex;
    }

    void VertexArray::AddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
    {
        glBindVertexArray(m_ID);
        vertexBuffer->Bind();

        m_vertexBuffers.push_back(vertexBuffer);

        for(const BufferElement& element : vertexBuffer->GetLayout())
        {
            size_t elementSizeDiv = element.Count / 4;
            size_t elementSizeRem = element.Count % 4;

            size_t offset = element.Offset;

            for(size_t i = 0; i < elementSizeDiv; i++)
            {
                AddAttribute(4, element.Type, vertexBuffer->GetLayout().GetStride(), offset, vertexBuffer->IsInstanced);
                offset += GetTypeSize(element.Type) * 4;
            }

            if(elementSizeRem > 0)
            {
                AddAttribute(elementSizeRem, element.Type, vertexBuffer->GetLayout().GetStride(), offset, vertexBuffer->IsInstanced);
                offset += GetTypeSize(element.Type) * elementSizeRem;
            }
        }
    }

    void VertexArray::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
    {
        glBindVertexArray(m_ID);
        indexBuffer->Bind();
        m_indexBuffer = indexBuffer;
    }

    void VertexArray::Draw(size_t vertexBufferIndex, DrawMode mode, size_t instanceCount) const
    {
        glBindVertexArray(m_ID);
        glDrawArraysInstanced((GLenum)mode, 0, GLsizei(m_vertexBuffers[vertexBufferIndex]->VertexCount), GLsizei(instanceCount));
    }

    void VertexArray::Draw(DrawMode mode, size_t instanceCount) const
    {
        glBindVertexArray(m_ID);

        if(m_indexBuffer)
            glDrawElementsInstanced((GLenum)mode, GLsizei(m_indexBuffer->IndexCount), GL_UNSIGNED_INT, nullptr, GLsizei(instanceCount));
    }
}