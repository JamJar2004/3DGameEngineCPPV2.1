#pragma once

#include <memory>

#include "Engine/Core/Array.hpp"

class DeviceRenderBuffer
{
public:
    virtual ~DeviceRenderBuffer() = default;

    virtual std::size_t SizeInBytes() const = 0;

    virtual void Update(ConstDynamicArraySlice instanceData) = 0;
};

using DeviceRenderBufferHandle = std::shared_ptr<DeviceRenderBuffer>;

template<typename TElement>
class LocalRenderBuffer
{
public:
    Array<TElement> Elements;

    void Update()
    {
        m_deviceBuffer->Update(Elements);
    }

    friend class RenderDevice;
private:
    explicit LocalRenderBuffer(const DeviceRenderBufferHandle& deviceBuffer, size_t elementCount) :
        Elements(elementCount), m_deviceBuffer(deviceBuffer) {}

    const DeviceRenderBufferHandle m_deviceBuffer;
};

template<typename TElement>
using LocalRenderBufferHandle = std::shared_ptr<LocalRenderBuffer<TElement>>;
