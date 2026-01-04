#pragma once

#include "../../Rendering/RenderDevice.hpp"

class OpenGLRenderDevice : public RenderDevice
{
public:
	explicit OpenGLRenderDevice(ScreenGraphicsMode graphicsMode);

	void Enable(uint32_t flags) override;

	void Disable(uint32_t flags) override;

	void SetFaceCullingMode(FaceCullingMode faceCullingMode) override;

	void SetDepthFunction(DepthFunction depthFunction) override;

	void SetBlendFunction(BlendFactor sourceFactor, BlendFactor destFactor) override;

	MeshHandle         CreateMesh        (const Model& model) override;
	TextureAtlasHandle CreateTextureAtlas(const BitmapBase& image, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) override;
	TextureAtlasHandle CreateCubeMap     (const BitmapBase& image, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) override;
	RenderTargetHandle CreateRenderTarget(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachment) override;
	ShaderHandle       CreateShader      (std::string_view sourceCode) override;

protected:
	DeviceRenderBufferHandle CreateDeviceRenderBuffer(size_t sizeInBytes) override;
private:
	ScreenGraphicsMode m_graphicsMode;
};