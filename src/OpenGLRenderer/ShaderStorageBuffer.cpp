
#include "ShaderStorageBuffer.hpp"

#include "GL/glew.h"

namespace GL
{
    ShaderStorageBuffer::~ShaderStorageBuffer()
    {
        glDeleteBuffers(1, &m_bufferId);
    }

    ShaderStorageBuffer::ShaderStorageBuffer(std::size_t sizeInBytes)
    {
        glCreateBuffers(1, &m_bufferId);
        glNamedBufferStorage(m_bufferId, sizeInBytes, nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    void ShaderStorageBuffer::Bind(uint32_t slot) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, m_bufferId);
    }

    void ShaderStorageBuffer::UpdateData(const void* data, size_t offset, size_t sizeInBytes)
    {
        glNamedBufferSubData(m_bufferId, offset, sizeInBytes, data);
    }
}
