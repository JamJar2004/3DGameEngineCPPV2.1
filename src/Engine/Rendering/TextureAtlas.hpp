#pragma once

#include <memory>
#include <memory.h>
#include <glm/glm.hpp>
#include <span>
#include <utility>

#include <Common.hpp>

#include "Reflection.hpp"

class TypeInfo;

enum class MinFilterMode
{
	Nearest,
	Linear,
	NearestMipmapNearest,
	NearestMipmapLinear,
	LinearMipmapNearest,
	LinearMipmapLinear,
};

enum class MagFilterMode
{
	Nearest,
	Linear,
};

enum class TextureWrappingMode
{
	Tiled,
	TiledMirrored,
	ClampedToEdge,
	ClampedToBorder,
	ClampedToEdgeMirrored,
};

enum class InternalImageFormat
{
	R8     ,
	R16    ,
	R32    ,
	R16F   ,
	R32F   ,
	RG8    ,
	RG16   ,
	RG32   ,
	RG16F  ,
	RG32F  ,
	RGB8   ,
	RGB16  ,
	RGB32  ,
	RGB16F ,
	RGB32F ,
	RGBA8  ,
	RGBA16 ,
	RGBA32 ,
	RGBA16F,
	RGBA32F,

	DepthComponent16,
	DepthComponent32,
	DepthComponent32F,
};

enum class ImageFormat
{
	R              = 1,
	RG            ,
	RGB           ,
	RGBA          ,
	DepthComponent,
	DepthStencil  ,
};

class BitmapBase;

using BitmapBaseHandle = std::shared_ptr<BitmapBase>;

using BitmapCreator = BitmapBaseHandle(*)(uint32_t width, uint32_t height, uint32_t faceCount);

template<typename TVector>
concept Vector = ShallowCopyable<TVector> && requires
{
	typename TVector::value_type;
}
&& requires(TVector vector)
{
	{ TVector::length() } -> std::same_as<glm::length_t>;
};

class PixelFormat
{
public:
	PixelFormat(const PixelFormat&) = delete;

	const InternalImageFormat BufferInternalFormat;

	const ImageFormat BufferFormat;

	TypeInfo* const PixelType;
	TypeInfo* const BufferElementType;

	const size_t ChannelCount;

	BitmapCreator CreateBitmap;

	template<Vector TPixel>
	static PixelFormat* Get();
private:
	template<Vector TPixel>
	static BitmapBaseHandle InternalCreateBitmap(uint32_t width, uint32_t height, uint32_t faceCount);

	template<ShallowCopyable TPixel>
	static PixelFormat* CreateFormat()
	{
		using TElement = TPixel::value_type;

		TypeInfo*   pixelType = TypeInfo::Get<TPixel>();
		TypeInfo* elementType = TypeInfo::Get<TElement>();

		InternalImageFormat internalFormat;
		ImageFormat format;

		switch(TPixel::length())
		{
			case 1:
				format = ImageFormat::R;
				break;
			case 2:
				format = ImageFormat::RG;
				break;
			case 3:
				format = ImageFormat::RGB;
				break;
			case 4:
				format = ImageFormat::RGBA;
				break;
			default:
				format = ImageFormat::RGBA;
		}

		if(pixelType == TypeInfo::Get<glm::u8vec1>())
		{
			internalFormat = InternalImageFormat::R8;
		}
		else if (pixelType == TypeInfo::Get<glm::u8vec2>())
		{
			internalFormat = InternalImageFormat::RG8;
		}

		return new PixelFormat(internalFormat, format, pixelType, elementType, TPixel::length(), InternalCreateBitmap<TPixel>);
	}

	PixelFormat(InternalImageFormat internalFormat, ImageFormat format, TypeInfo* type, TypeInfo* elementType, size_t channelCount, BitmapCreator creator) :
		BufferInternalFormat(internalFormat),
		BufferFormat(format),
		PixelType(type),
		BufferElementType(elementType),
		ChannelCount(channelCount),
		CreateBitmap(creator) {}


	template<Vector TPixel>
	struct Storage
	{
		inline static PixelFormat* s_format = CreateFormat<TPixel>();
	};
};

template<Vector TPixel>
PixelFormat* PixelFormat::Get() { return Storage<TPixel>::s_format; }

class BitmapBase
{
public:
	BitmapBase(uint32_t width, uint32_t height, uint32_t faceCount, PixelFormat* format) : Width(width), Height(height), FaceCount(faceCount), PixelCount(size_t(width) * height * faceCount), Format(format) {}

	virtual ~BitmapBase() = default;

	const uint32_t     Width;
	const uint32_t     Height;
	const uint32_t     FaceCount;
	const size_t       PixelCount;
	PixelFormat* const Format;

	[[nodiscard]] glm::uvec2 Size() const { return {Width, Height}; }

	void* GetFacePixels(size_t face)
	{
		const auto pixels = static_cast<uint8_t*>(GetPixels());
		return pixels + PixelCount * Format->PixelType->Size * face;
	}

				  virtual       void* GetPixels()       = 0;
	[[nodiscard]] virtual const void* GetPixels() const = 0;
};

template<typename TConverter, typename TSource, typename TTarget>
concept PixelConverter = requires(TConverter value, TSource source)
{
	{ value(source) } -> std::same_as<TTarget>;
};



template<Vector TPixel>
class Bitmap final : public BitmapBase
{
public:
	template<Vector TSourcePixel, PixelConverter<TSourcePixel, TPixel> Converter>
	Bitmap(const Bitmap<TSourcePixel>& source, Converter converter) :
		BitmapBase(source.Width, source.Height, source.FaceCount, PixelFormat::Get<TPixel>()),
		Pixels(new TPixel[static_cast<size_t>(source.Width) * source.Height * source.FaceCount])
	{
		for(size_t i = 0; i < source.PixelCount; i++)
		{
			Pixels[i] = converter(source.Pixels[i]);
		}
	}

	Bitmap(uint32_t width, uint32_t height, uint32_t faceCount = 1) : BitmapBase(width, height, faceCount, PixelFormat::Get<TPixel>()), Pixels(new TPixel[size_t(width) * height * faceCount]) {}

	Bitmap(const Bitmap& other) : Bitmap(other.Width, other.Height, other.FaceCount)
	{
		other.CopyTo(*this);
	}

	TPixel* const Pixels;

						void* GetPixels()		override { return Pixels; }
	[[nodiscard]] const void* GetPixels() const override { return Pixels; }

						TPixel& GetPixel(uint32_t x, uint32_t y, uint32_t face = 0)       { return Pixels[PixelCount * face + (static_cast<size_t>(y) * Width + x)]; }
	[[nodiscard]] const TPixel& GetPixel(uint32_t x, uint32_t y, uint32_t face = 0) const { return Pixels[PixelCount * face + (static_cast<size_t>(y) * Width + x)]; }

						TPixel& GetPixel(const glm::uvec2& location, uint32_t face = 0)       { return GetPixel(location.x, location.y, face); }
	[[nodiscard]] const TPixel& GetPixel(const glm::uvec2& location, uint32_t face = 0) const { return GetPixel(location.x, location.y, face); }

	void ZeroPixels()
	{
		memset(Pixels, 0, PixelCount * Format->PixelType->Size);
	}

	void CopyTo(Bitmap& target) const
	{
		memcpy(target.Pixels, Pixels, PixelCount * sizeof(TPixel));
	}
};



template<Vector TPixel>
BitmapBaseHandle PixelFormat::InternalCreateBitmap(uint32_t width, uint32_t height, uint32_t faceCount) { return std::make_shared<Bitmap<TPixel>>(width, height, faceCount); }

template<Vector TPixel>
using BitmapHandle = std::shared_ptr<Bitmap<TPixel>>;

class TextureHandle;

struct Rectangle
{
	explicit Rectangle(const glm::vec2& offset = glm::vec2(1.0f), const glm::vec2& size = glm::vec2(1.0f)) : Offset(offset), Size(size) {}

	glm::vec2 Offset;
	glm::vec2 Size;
};

class RenderDevice;

class TextureAtlas
{
public:
	TextureAtlas(uint32_t width, uint32_t height) :
		Width(width), Height(height), m_bindingIndex(0), m_renderDevice(nullptr) {}

	virtual ~TextureAtlas();

	union
	{
		struct
		{
			const uint32_t Width;
			const uint32_t Height;
		};
		const glm::uvec2 Size{};
	};
protected:
	virtual void Bind(uint32_t bindingIndex) = 0;

	friend class RenderDevice;
private:
	uint32_t m_bindingIndex;
	RenderDevice* m_renderDevice;
};


using TextureAtlasHandle = std::shared_ptr<TextureAtlas>;

class TextureHandle
{
public:
	explicit TextureHandle(TextureAtlasHandle atlas) : Atlas(std::move(atlas)), Region(Rectangle(glm::vec2(0), atlas->Size)) {}

	TextureHandle(TextureAtlasHandle atlas, const Rectangle& region) :
		Atlas(std::move(atlas)),
		Region(region) {}

	TextureAtlasHandle Atlas;
	Rectangle Region;
};

