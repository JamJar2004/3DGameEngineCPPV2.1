#pragma once

#include <optional>

#include "Mesh.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "FrameBuffer.hpp"

struct GraphicsMode
{
	GraphicsMode(uint32_t width, uint32_t height) : Width(width), Height(height) {}

	uint32_t Width;
	uint32_t Height;

	float GetAspectRatio() const { return Width / float(Height); }
};

enum class FaceCullingMode
{
	None, Inside, Outside,
};

enum class BlendFactor
{
	Zero,
	One,
	SourceColor,
	DestColor,
	OneMinusSourceColor,
	OneMinusDestColor,
	SourceAlpha,
	DestAlpha,
	OneMinusSourceAlpha,
	OneMinusDestAlpha,
};

enum class DepthFunction
{
	Never,
	Equal,
	NotEqual,
	Smaller,
	Greater,
	SmallerOrEqual,
	GreaterOrEqual,
	Always
};

enum RenderFlags : uint32_t
{
	Blending     = 1,
	DepthTest    = 2,
	DepthClamp   = 4,
	DepthWriting = 8,
	MultiSample  = 16,
	ClipPlane0   = 32,
	ClipPlane1   = 64,
	ClipPlane2   = 128,
	ClipPlane3   = 256,
	ClipPlane4   = 512,
	ClipPlane5   = 1024,
};

class RenderDevice
{
public:
	RenderDevice(FrameBufferHandle screenBuffer) : ScreenBuffer(screenBuffer) {}

	virtual ~RenderDevice() {}

	const FrameBufferHandle ScreenBuffer;

	virtual void Enable(uint32_t flags) = 0;
	virtual void Disable(uint32_t flags) = 0;

	virtual void SetClearColor(const glm::vec4& color) = 0;
	virtual void ClearScreen(int bufferType) = 0;

	virtual void ClearColorBuffer(PixelFormat format, const void* value) = 0;

	virtual void SetFaceCullingMode(FaceCullingMode faceCullingMode) = 0;

	virtual void SetDepthFunction(DepthFunction depthFunction) = 0;
	virtual void SetBlendFunction(BlendFactor sourceFactor, BlendFactor destFactor) = 0;

	virtual MeshHandle        CreateMesh       (const Model& model) = 0;
	virtual TextureHandle     CreateTexture    (const BaseBitmap& image, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode) = 0;
	virtual TextureHandle     CreateCubeMap    (const BaseBitmap& image, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode) = 0;
	virtual FrameBufferHandle CreateFrameBuffer(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachment) = 0;
	virtual ShaderHandle      CreateShader     (const std::string& vertexShaderText, const std::string& fragmentShaderText) = 0;
};