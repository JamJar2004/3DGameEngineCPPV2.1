#pragma once

#include <memory>
#include <utility>

#include "Engine/Core/Buffer.hpp"

class DeviceRenderBuffer
{
public:
    virtual ~DeviceRenderBuffer() = default;

    [[nodiscard]] virtual std::size_t SizeInBytes() const = 0;

    virtual void Update(ConstDynamicBufferSlice instanceData) = 0;
};

using DeviceRenderBufferHandle = std::shared_ptr<DeviceRenderBuffer>;

template<ShallowCopyable TElement>
class LocalRenderBuffer
{
public:
    Buffer<TElement> Elements;

    explicit LocalRenderBuffer(DeviceRenderBufferHandle deviceBuffer, size_t elementCount, const TElement& defaultValue) :
        Elements(elementCount, defaultValue), m_deviceBuffer(std::move(deviceBuffer)) {}

    void Update(size_t count)
    {
        m_deviceBuffer->Update(Elements.AsSlice(0, count));
    }

    friend class RenderDevice;
private:
    const DeviceRenderBufferHandle m_deviceBuffer;
};

template<ShallowCopyable TElement>
using LocalRenderBufferHandle = std::shared_ptr<LocalRenderBuffer<TElement>>;
