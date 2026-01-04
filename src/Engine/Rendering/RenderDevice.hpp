#pragma once

#include <optional>
#include <queue>
#include <utility>

#include "Mesh.hpp"
#include "RenderBuffer.hpp"
#include "TextureAtlas.hpp"
#include "Shader.hpp"
#include "RenderTarget.hpp"

struct ScreenGraphicsMode
{
	ScreenGraphicsMode(uint32_t width, uint32_t height) : Width(width), Height(height) {}

	uint32_t Width;
	uint32_t Height;

	[[nodiscard]] float GetAspectRatio() const { return Width / static_cast<float>(Height); }
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
	explicit RenderDevice(RenderTargetHandle screenBuffer, size_t maximumTextureBindCount) :
		ScreenBuffer(std::move(screenBuffer)),
		BoundRenderTarget(std::move(screenBuffer))
	{
		m_boundTextures.resize(maximumTextureBindCount, {});
		m_boundTextureEndIndex = 0;
	}

	virtual ~RenderDevice() = default;

	const RenderTargetHandle ScreenBuffer;

	virtual void Enable(uint32_t flags) = 0;
	virtual void Disable(uint32_t flags) = 0;

	virtual void SetFaceCullingMode(FaceCullingMode faceCullingMode) = 0;

	virtual void SetDepthFunction(DepthFunction depthFunction) = 0;
	virtual void SetBlendFunction(BlendFactor sourceFactor, BlendFactor destFactor) = 0;

	template<typename TElement>
	LocalRenderBufferHandle<TElement> CreateRenderBuffer(size_t elementCount, const TElement& defaultValue)
	{
		return std::make_shared<LocalRenderBuffer<TElement>>(CreateDeviceRenderBuffer(elementCount * sizeof(TElement)), elementCount, defaultValue);
	}

	virtual MeshHandle         CreateMesh        (const Model& model) = 0;
	virtual TextureAtlasHandle CreateTextureAtlas(const BitmapBase& image, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) = 0;
	virtual TextureAtlasHandle CreateCubeMap     (const BitmapBase& image, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode) = 0;
	virtual RenderTargetHandle CreateRenderTarget(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachment) = 0;
	virtual ShaderHandle       CreateShader      (std::string_view sourceCode) = 0;

	friend class RenderTarget;
	friend class MaterialTextureInfo;
	friend class TextureAtlas;
protected:
	virtual DeviceRenderBufferHandle CreateDeviceRenderBuffer(size_t sizeInBytes) = 0;
private:
	RenderTargetHandle BoundRenderTarget;

	std::vector<std::weak_ptr<TextureAtlas>> m_boundTextures;
	std::queue<uint32_t> m_freeSpaces;

	uint32_t m_boundTextureEndIndex;

	uint32_t BindTexture(const TextureAtlasHandle& atlas)
	{
		uint32_t result;
		if (!m_freeSpaces.empty())
		{
			result = m_freeSpaces.front();
			m_freeSpaces.pop();
		}
		else
		{
			result = m_boundTextureEndIndex++;
		}

		atlas->m_renderDevice = this;
		atlas->m_bindingIndex = result;
		atlas->Bind(result);
		m_boundTextures[result] = atlas;
		return result;
	}

	void DeleteTexture(uint32_t index)
	{
		if (index == m_boundTextureEndIndex - 1)
		{
			--m_boundTextureEndIndex;
		}
		else
		{
			m_freeSpaces.push(index);
		}
	}
};