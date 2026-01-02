#include "OpenGLRenderBuffer.hpp"

std::size_t OpenGLRenderBuffer::SizeInBytes() const
{
    return m_sizeInBytes;
}

void OpenGLRenderBuffer::Update(ConstDynamicBufferSlice instanceData)
{
    m_buffer.UpdateData(instanceData.Data(), 0, instanceData.Count());
}
