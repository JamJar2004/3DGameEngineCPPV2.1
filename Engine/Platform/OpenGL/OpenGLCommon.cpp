#include "OpenGLCommon.hpp"

GL::ElementType GetElementType(ElementType elementType)
{
    switch(elementType)
    {
    case ElementType::UInt8:
        return GL::ElementType::UInt8;
    case ElementType::UInt16:
        return GL::ElementType::UInt16;
    case ElementType::UInt32:
        return GL::ElementType::UInt32;
    case ElementType::SInt8:
        return GL::ElementType::SInt8;
    case ElementType::SInt16:
        return GL::ElementType::SInt16;
    case ElementType::SInt32:
        return GL::ElementType::SInt32;
    case ElementType::Float32:
        return GL::ElementType::Float32;
    case ElementType::Float64:
        return GL::ElementType::Float64;
    }

    return GL::ElementType::UInt8;
}