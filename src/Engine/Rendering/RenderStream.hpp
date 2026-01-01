#pragma once

#include <utility>

#include "Mesh.hpp"
#include "RenderBuffer.hpp"
#include "RenderTarget.hpp"

template<ShallowCopyable TElement>
class RenderStream final
{
public:
    explicit RenderStream(
        const LocalRenderBufferHandle<TElement>& renderBuffer,
        const MeshHandle& mesh,
        const ShaderHandle& shader,
        const RenderTargetHandle& renderTarget) :
            m_renderBuffer(renderBuffer),
            m_mesh(mesh),
            m_shader(shader),
            m_renderTarget(renderTarget),
            m_index(0) {}

    void Write(const TElement& element)
    {
        if (m_index >= m_renderBuffer->Elements.Count())
        {
            Flush();
        }

        m_renderBuffer->Elements[m_index++] = element;
    }

    template<typename... TArgs>
    TElement& WriteEmplace(TArgs... args) requires std::constructible_from<TElement, TArgs...>
    {
        if (m_index >= m_renderBuffer->Elements.Count())
        {
            Flush();
        }

        return *new(&m_renderBuffer->Elements[m_index++]) TElement(std::forward<TArgs>(args)...);
    }

    void WriteRange(ConstBufferSlice<TElement> elements)
    {
        auto elementsToWrite = elements;

        auto remainingBuffer = GetRemainingBuffer();
        while (elementsToWrite.Count() > remainingBuffer.Count())
        {
            if (remainingBuffer.Count() > 0)
            {
                elementsToWrite.Slice(0, remainingBuffer.Count()).CopyTo(remainingBuffer);
                elementsToWrite = elementsToWrite.Slice(remainingBuffer.Count());
            }
            Flush();
            remainingBuffer = GetRemainingBuffer();
        }

        elementsToWrite.CopyTo(remainingBuffer);
    }

    void Flush()
    {
        size_t instanceCount = m_index;
        m_index = 0;
        m_shader->Use();
        m_renderTarget->Use();
        m_renderBuffer->Update(instanceCount);
        m_mesh->Draw(instanceCount);
    }
private:
    LocalRenderBufferHandle<TElement> m_renderBuffer;

    MeshHandle m_mesh;

    ShaderHandle m_shader;

    RenderTargetHandle m_renderTarget;

    size_t m_index;

    BufferSlice<TElement> GetWrittenBuffer() const { return m_renderBuffer->Elements.AsSlice(0, m_index); }

    BufferSlice<TElement> GetRemainingBuffer() const { return m_renderBuffer->Elements.AsSlice(m_index); }
};
