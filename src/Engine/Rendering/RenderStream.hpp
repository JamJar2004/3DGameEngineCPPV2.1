#pragma once

#include <utility>

#include "Mesh.hpp"
#include "RenderBuffer.hpp"
#include "RenderTarget.hpp"

template<typename TElement>
class RenderStream final
{
public:
    explicit RenderStream(
        const LocalRenderBufferHandle<TElement>& renderBuffer,
        const MeshHandle& mesh,
        const RenderTargetHandle& renderTarget) :
            m_renderBuffer(renderBuffer),
            m_mesh(std::move(mesh)),
            m_renderTarget(renderTarget),
            m_index(0) {}

    void Write(const TElement& element)
    {
        if (m_index >= m_renderBuffer->Elements->Count())
        {
            Flush();
        }

        m_renderBuffer->Elements[m_index++] = element;
    }

    void WriteRange(ConstArraySlice<TElement> elements)
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
        m_renderTarget->Bind();
        m_renderBuffer->Update(GetWrittenBuffer());
        m_mesh->Draw();
        m_index = 0;
    }
private:
    LocalRenderBufferHandle<TElement> m_renderBuffer;

    MeshHandle m_mesh;

    RenderTargetHandle m_renderTarget;

    size_t m_index;

    ArraySlice<TElement> GetWrittenBuffer() const { return m_renderBuffer->Elements.AsSlice(0, m_index); }

    ArraySlice<TElement> GetRemainingBuffer() const { return m_renderBuffer->Elements.AsSlice(m_index); }
};
