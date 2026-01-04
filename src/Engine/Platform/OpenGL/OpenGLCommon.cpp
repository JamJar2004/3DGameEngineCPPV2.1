#include "OpenGLCommon.hpp"

#include "Reflection.hpp"

class TypeInfo;

GL::ElementType GetElementType(TypeInfo* elementType)
{
    if (elementType == TypeInfo::Get<uint8_t>())
    {
        return GL::ElementType::UInt8;
    }

    if (elementType == TypeInfo::Get<uint16_t>())
    {
        return GL::ElementType::UInt16;
    }

    if (elementType == TypeInfo::Get<uint32_t>())
    {
        return GL::ElementType::UInt32;
    }

    if (elementType == TypeInfo::Get<int8_t>())
    {
        return GL::ElementType::SInt8;
    }

    if (elementType == TypeInfo::Get<int16_t>())
    {
        return GL::ElementType::SInt16;
    }

    if (elementType == TypeInfo::Get<int32_t>())
    {
        return GL::ElementType::SInt32;
    }

    if (elementType == TypeInfo::Get<float>())
    {
        return GL::ElementType::Float32;
    }

    if (elementType == TypeInfo::Get<int32_t>())
    {
        return GL::ElementType::Float64;
    }

    return GL::ElementType::UInt8;
}
