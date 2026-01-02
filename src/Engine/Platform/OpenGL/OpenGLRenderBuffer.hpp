#pragma once
#include "Engine/Rendering/RenderBuffer.hpp"
#include "OpenGLRenderer/ShaderStorageBuffer.hpp"

class OpenGLRenderBuffer final : public DeviceRenderBuffer
{
public:
    explicit OpenGLRenderBuffer(size_t sizeInBytes) : m_buffer(sizeInBytes), m_sizeInBytes(sizeInBytes) {}

    [[nodiscard]] std::size_t SizeInBytes() const override;

    void Update(ConstDynamicBufferSlice instanceData) override;
private:
    GL::ShaderStorageBuffer m_buffer;

    const size_t m_sizeInBytes;
};
