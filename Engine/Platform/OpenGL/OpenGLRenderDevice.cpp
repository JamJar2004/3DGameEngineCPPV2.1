#include "OpenGLRenderDevice.hpp"

#include "OpenGLMesh.hpp"
#include "OpenGLTexture.hpp"
#include "OpenGLFrameBuffer.hpp"
#include "OpenGLShader.hpp"

#include "../../Core/Application.hpp"

#include <iostream>
#include <array>

#include <OpenGLRenderer/Renderer.hpp>

OpenGLRenderDevice::OpenGLRenderDevice(GraphicsMode graphicsMode) :
    RenderDevice(std::make_shared<OpenGLFrameBuffer>(graphicsMode.Width, graphicsMode.Height)),
    m_graphicsMode(graphicsMode)
{
	if(glewInit() != GLEW_OK)
		std::cout << "Glew Initialization Failed!" << std::endl;

	GL::Enable(GL::EnableFlags::CullFace);
	GL::Enable(GL::EnableFlags::DepthTest);
	GL::Enable(GL::EnableFlags::DepthClamp);
	//GL::Enable(GL::EnableFlags::MultiSample);

    GL::FrontFace(GL::FrontFaceMode::Clockwise);
	GL::CullFace(GL::CullFaceMode::Back);

	GL::SetClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

void OpenGLRenderDevice::Enable(uint32_t flags)
{
    if(flags & RenderFlags::Blending)
        GL::Enable(GL::EnableFlags::Blend);

    if(flags & RenderFlags::DepthTest)
        GL::Enable(GL::EnableFlags::DepthTest);

    if(flags & RenderFlags::DepthClamp)
        GL::Enable(GL::EnableFlags::DepthClamp);

    if(flags & RenderFlags::DepthWriting)
        GL::DepthMask(true);

    if(flags & RenderFlags::MultiSample)
        GL::Enable(GL::EnableFlags::MultiSample);

    if(flags & RenderFlags::ClipPlane0)
        GL::Enable(GL::EnableFlags::ClipPlane0);

    if(flags & RenderFlags::ClipPlane1)
        GL::Enable(GL::EnableFlags::ClipPlane1);

    if(flags & RenderFlags::ClipPlane2)
        GL::Enable(GL::EnableFlags::ClipPlane2);

    if(flags & RenderFlags::ClipPlane3)
        GL::Enable(GL::EnableFlags::ClipPlane3);

    if(flags & RenderFlags::ClipPlane4)
        GL::Enable(GL::EnableFlags::ClipPlane4);

    if(flags & RenderFlags::ClipPlane5)
        GL::Enable(GL::EnableFlags::ClipPlane5);
}

void OpenGLRenderDevice::Disable(uint32_t flags)
{
    if(flags & RenderFlags::Blending)
        GL::Disable(GL::EnableFlags::Blend);

    if(flags & RenderFlags::DepthTest)
        GL::Disable(GL::EnableFlags::DepthTest);

    if(flags & RenderFlags::DepthClamp)
        GL::Disable(GL::EnableFlags::DepthClamp);

    if(flags & RenderFlags::DepthWriting)
        GL::DepthMask(false);

    if(flags & RenderFlags::MultiSample)
        GL::Disable(GL::EnableFlags::MultiSample);

    if(flags & RenderFlags::ClipPlane0)
        GL::Disable(GL::EnableFlags::ClipPlane0);

    if(flags & RenderFlags::ClipPlane1)
        GL::Disable(GL::EnableFlags::ClipPlane1);

    if(flags & RenderFlags::ClipPlane2)
        GL::Disable(GL::EnableFlags::ClipPlane2);

    if(flags & RenderFlags::ClipPlane3)
        GL::Disable(GL::EnableFlags::ClipPlane3);

    if(flags & RenderFlags::ClipPlane4)
        GL::Disable(GL::EnableFlags::ClipPlane4);

    if(flags & RenderFlags::ClipPlane5)
        GL::Disable(GL::EnableFlags::ClipPlane5);
}

//void OpenGLRenderDevice::BindPrimaryFrameBuffer()
//{
//    glViewport(0, 0, m_graphicsMode.Width, m_graphicsMode.Height);
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}

void OpenGLRenderDevice::SetClearColor(const glm::vec4& color)
{
    GL::SetClearColor(color);
}

void OpenGLRenderDevice::ClearScreen(int bufferType)
{
	uint32_t mask = 0;

	if(bufferType & ScreenBufferType::ColorBuffer)
		mask |= GL::BufferTarget::ColorBuffer;

	if(bufferType & ScreenBufferType::DepthBuffer)
        mask |= GL::BufferTarget::DepthBuffer;

	if(bufferType & ScreenBufferType::StencilBuffer)
		mask |= GL::BufferTarget::StencilBuffer;

	GL::Clear(mask);
}

void OpenGLRenderDevice::ClearColorBuffer(PixelFormat format, const void* value)
{
    switch(format.BufferInternalFormat)
	{
        case InternalFormat::R32     : GL::ClearUIntBuffer (*(glm::uvec1*)value); break;
        case InternalFormat::R32F    : GL::ClearFloatBuffer(*(glm::fvec1*)value); break;
		case InternalFormat::RG32    : GL::ClearUIntBuffer (*(glm::uvec2*)value); break;
		case InternalFormat::RG32F   : GL::ClearFloatBuffer(*(glm::fvec2*)value); break;
		case InternalFormat::RGB32   : GL::ClearUIntBuffer (*(glm::uvec3*)value); break;
		case InternalFormat::RGB32F  : GL::ClearFloatBuffer(*(glm::fvec3*)value); break;
		case InternalFormat::RGBA32  : GL::ClearUIntBuffer (*(glm::uvec4*)value); break;
        case InternalFormat::RGBA32F : GL::ClearFloatBuffer(*(glm::fvec4*)value); break;
	}
}

void OpenGLRenderDevice::SetFaceCullingMode(FaceCullingMode faceCullingMode)
{
	if(faceCullingMode == FaceCullingMode::None)
		GL::Disable(GL::EnableFlags::CullFace);
	else
	{
		GL::Enable(GL::EnableFlags::CullFace);
		if(faceCullingMode == FaceCullingMode::Inside)
			GL::CullFace(GL::CullFaceMode::Back);
		else if(faceCullingMode == FaceCullingMode::Outside)
            GL::CullFace(GL::CullFaceMode::Front);
	}
}

GL::BlendFactor GetBlendFactor(BlendFactor blendFactor)
{
    switch(blendFactor)
    {
		case BlendFactor::Zero:
            return GL::BlendFactor::Zero;
		case BlendFactor::One:
            return GL::BlendFactor::One;
		case BlendFactor::SourceAlpha:
            return GL::BlendFactor::SourceAlpha;
		case BlendFactor::SourceColor:
            return GL::BlendFactor::SourceColor;
        case BlendFactor::DestAlpha:
            return GL::BlendFactor::DestAlpha;
        case BlendFactor::DestColor:
            return GL::BlendFactor::DestColor;
        case BlendFactor::OneMinusSourceAlpha:
            return GL::BlendFactor::OneMinusSourceAlpha;
		case BlendFactor::OneMinusSourceColor:
            return GL::BlendFactor::OneMinusSourceColor;
		case BlendFactor::OneMinusDestAlpha:
            return GL::BlendFactor::OneMinusDestAlpha;
        case BlendFactor::OneMinusDestColor:
            return GL::BlendFactor::OneMinusDestColor;
    }
    return GL::BlendFactor::One;
}

void OpenGLRenderDevice::SetDepthFunction(DepthFunction depthFunction)
{
	switch(depthFunction)
    {
        case DepthFunction::Never:
            GL::DepthFunc(GL::DepthFunction::Never);
            break;
        case DepthFunction::Smaller:
            GL::DepthFunc(GL::DepthFunction::Less);
            break;
        case DepthFunction::SmallerOrEqual:
            GL::DepthFunc(GL::DepthFunction::LessOrEqual);
            break;
        case DepthFunction::Equal:
            GL::DepthFunc(GL::DepthFunction::Equal);
            break;
        case DepthFunction::NotEqual:
            GL::DepthFunc(GL::DepthFunction::NotEqual);
            break;
        case DepthFunction::GreaterOrEqual:
            GL::DepthFunc(GL::DepthFunction::GreaterOrEqual);
            break;
        case DepthFunction::Greater:
            GL::DepthFunc(GL::DepthFunction::Greater);
            break;
        case DepthFunction::Always:
            GL::DepthFunc(GL::DepthFunction::Always);
            break;
    }
}

void OpenGLRenderDevice::SetBlendFunction(BlendFactor sourceFactor, BlendFactor destFactor)
{
	GL::BlendFactor source = GetBlendFactor(sourceFactor);
    GL::BlendFactor dest   = GetBlendFactor(destFactor);
	GL::BlendFunc(source, dest);
}

MeshHandle OpenGLRenderDevice::CreateMesh(const Model& model)
{
    return std::make_shared<OpenGLMesh>(model);
}

TextureHandle OpenGLRenderDevice::CreateTexture(const BaseBitmap& image, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode)
{
    return std::make_shared<OpenGLTexture>(image, minFilter, magFilter, wrappingMode);
}

TextureHandle OpenGLRenderDevice::CreateCubeMap(const BaseBitmap& image, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode)
{
    return std::make_shared<OpenGLTextureCube>(image, minFilter, magFilter, wrappingMode);
}

FrameBufferHandle OpenGLRenderDevice::CreateFrameBuffer(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachment)
{
    return std::make_shared<OpenGLFrameBuffer>(width, height, colorAttachments, depthAttachment);
}


ShaderHandle OpenGLRenderDevice::CreateShader(const std::string& vertexShaderText, const std::string& fragmentShaderText)
{
	return std::make_shared<OpenGLShader>(vertexShaderText, fragmentShaderText);
}


