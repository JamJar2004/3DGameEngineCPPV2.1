#pragma once

#include "../../Rendering/RenderDevice.hpp"

class OpenGLRenderDevice : public RenderDevice
{
private:
	GraphicsMode m_graphicsMode;
public:
	OpenGLRenderDevice(GraphicsMode graphicsMode);

	virtual void Enable(uint32_t flags) override;
	virtual void Disable(uint32_t flags) override;

	virtual void SetClearColor(const glm::vec4& color) override;

	virtual void ClearScreen(int bufferType) override;

	virtual void ClearColorBuffer(PixelFormat format, const void* value) override;

	virtual void SetFaceCullingMode(FaceCullingMode faceCullingMode) override;

	virtual void SetDepthFunction(DepthFunction depthFunction) override;
	virtual void SetBlendFunction(BlendFactor sourceFactor, BlendFactor destFactor) override;

	virtual MeshHandle        CreateMesh       (const Model& model) override;
	virtual TextureHandle     CreateTexture    (const BaseBitmap& image, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode) override;
	virtual TextureHandle     CreateCubeMap    (const BaseBitmap& image, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode) override;
	virtual FrameBufferHandle CreateFrameBuffer(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachment) override;
	virtual ShaderHandle      CreateShader     (const std::string& vertexShaderText, const std::string& fragmentShaderText) override;
	
};