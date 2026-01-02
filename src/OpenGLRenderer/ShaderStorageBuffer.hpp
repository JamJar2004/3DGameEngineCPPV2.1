#pragma once

#include "GL/glew.h"

namespace GL
{
    class ShaderStorageBuffer
    {
    public:
        explicit ShaderStorageBuffer(std::size_t sizeInBytes);

        ShaderStorageBuffer(const ShaderStorageBuffer&) = default;

        ~ShaderStorageBuffer();

        ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = default;

        void Bind(uint32_t slot) const;

        void UpdateData(const void* data, size_t offset, size_t sizeInBytes);
    private:
        GLuint m_bufferId;
    };
}

