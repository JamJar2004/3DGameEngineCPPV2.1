#pragma once

#include <stdint.h>
#include <memory>
#include <glm/glm.hpp>
#include <span>
#include <functional>

#include "Common.hpp"

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

enum class WrappingMode
{
	Tiled,
	TiledMirrored,
	ClampedToEdge,
	ClampedToBorder,
	ClampedToEdgeMirrored,
};

enum class InternalFormat
{
	DepthComponent16 ,
	DepthComponent32 ,
	DepthComponent32F,
	R8               ,
	R16              ,
	R32              ,
	R16F             ,
	R32F             ,
	RG8              ,
	RG16             ,
	RG32             ,
	RG16F            ,
	RG32F            ,
	RGB8             ,
	RGB16            ,
	RGB32            ,
	RGB16F           ,
	RGB32F           ,
	RGBA8            ,
	RGBA16           ,
	RGBA32           ,
	RGBA16F          ,
	RGBA32F          ,
};

enum class Format
{
	DepthComponent,
	DepthStencil  ,
	R             ,
	RG            ,
	RGB           ,
	RGBA          ,
};

class BaseBitmap;

using BaseBitmapHandle = std::shared_ptr<BaseBitmap>;

using BitmapCreator = BaseBitmapHandle(*)(uint32_t width, uint32_t height, uint32_t faceCount);

class PixelFormat
{
private:
	template<typename Pixel>
	static BaseBitmapHandle InternalCreateBitmap(uint32_t width, uint32_t height, uint32_t faceCount);

	template<typename Pixel>
	static PixelFormat CreateFormat(InternalFormat internalFormat, Format format, ElementType elementType, size_t channelCount)
	{
		return PixelFormat(internalFormat, format, elementType, sizeof(Pixel), channelCount, InternalCreateBitmap<Pixel>);
	}
	
	PixelFormat(InternalFormat internalFormat, Format format, ElementType elementType, size_t size, size_t channelCount, BitmapCreator creator) : BufferInternalFormat(internalFormat), BufferFormat(format), BufferElementType(elementType), Size(size), ChannelCount(channelCount), CreateBitmap(creator) {}
public:
	template<typename Pixel>
	static PixelFormat GetFormat();

	template<>
	static PixelFormat GetFormat<uint8_t>()  { return CreateFormat<uint8_t>(InternalFormat::R8, Format::R, ElementType::UInt8, 1); }
	template<>
	static PixelFormat GetFormat<uint16_t>() { return CreateFormat<uint16_t>(InternalFormat::R16, Format::R, ElementType::UInt16, 1); }
	template<>
	static PixelFormat GetFormat<uint32_t>() { return CreateFormat<uint32_t>(InternalFormat::R32, Format::R, ElementType::UInt32, 1); }
	template<>
	static PixelFormat GetFormat<float>()    { return CreateFormat<float>(InternalFormat::R32F, Format::R, ElementType::Float32, 1); }

	template<>
	static PixelFormat GetFormat<glm::u8vec2>() { return CreateFormat<glm::u8vec2>(InternalFormat::RG8, Format::RG, ElementType::UInt8, 2); }
	template<>
	static PixelFormat GetFormat<glm::u16vec2>() { return CreateFormat<glm::u16vec2>(InternalFormat::RG16, Format::RG, ElementType::UInt16, 2); }
	template<>
	static PixelFormat GetFormat<glm::u32vec2>() { return CreateFormat<glm::u32vec2>(InternalFormat::RG32, Format::RG, ElementType::UInt32, 2); }
	template<>
	static PixelFormat GetFormat<glm::fvec2>() { return CreateFormat<glm::fvec2>(InternalFormat::RG32F, Format::RG, ElementType::Float32, 2); }

	template<>
	static PixelFormat GetFormat<glm::u8vec3>() { return CreateFormat<glm::u8vec3>(InternalFormat::RGB8, Format::RGB, ElementType::UInt8, 3); }
	template<>
	static PixelFormat GetFormat<glm::u16vec3>() { return CreateFormat<glm::u16vec3>(InternalFormat::RGB16, Format::RGB, ElementType::UInt16, 3); }
	template<>
	static PixelFormat GetFormat<glm::u32vec3>() { return CreateFormat<glm::u32vec3>(InternalFormat::RGB32, Format::RGB, ElementType::UInt32, 3); }
	template<>
	static PixelFormat GetFormat<glm::fvec3>() { return CreateFormat<glm::fvec3>(InternalFormat::RGB32F, Format::RGB, ElementType::Float32, 3); }

	template<>
	static PixelFormat GetFormat<glm::u8vec4>() { return CreateFormat<glm::u8vec4>(InternalFormat::RGBA8, Format::RGBA, ElementType::UInt8, 4); }
	template<>
	static PixelFormat GetFormat<glm::u16vec4>() { return CreateFormat<glm::u16vec4>(InternalFormat::RGBA16, Format::RGBA, ElementType::UInt16, 4); }
	template<>
	static PixelFormat GetFormat<glm::u32vec4>() { return CreateFormat<glm::u32vec4>(InternalFormat::RGBA32, Format::RGBA, ElementType::UInt32, 4); }
	template<>
	static PixelFormat GetFormat<glm::fvec4>() { return CreateFormat<glm::fvec4>(InternalFormat::RGBA32F, Format::RGBA, ElementType::Float32, 4); }

	const InternalFormat BufferInternalFormat;

	const Format BufferFormat;

	const ElementType BufferElementType;

	const size_t Size;
	const size_t ChannelCount;

	BitmapCreator CreateBitmap;
};

class BaseBitmap
{
public:
	BaseBitmap(uint32_t width, uint32_t height, uint32_t faceCount, PixelFormat format) : Width(width), Height(height), FaceCount(faceCount), PixelCount(size_t(width) * height * faceCount), Format(format) {}

	const uint32_t Width;
	const uint32_t Height;
	const uint32_t FaceCount;
	const size_t   PixelCount;
	const PixelFormat Format;

	glm::uvec2 Size() const { return glm::uvec2(Width, Height); }

	void* GetFacePixels(size_t face)
	{
		uint8_t* pixels = (uint8_t*)GetPixels();
		return pixels + PixelCount * Format.Size * face;
	}

	virtual       void* GetPixels()       = 0;
	virtual const void* GetPixels() const = 0;
};

template<typename T, typename SourceType, typename TargetType>
concept PixelConverter = requires(T value, SourceType source)
{
	{ value(source) } -> std::same_as<TargetType>;
};

template<typename Pixel>
class Bitmap : public BaseBitmap
{
public:
	template<typename SourcePixel, PixelConverter<SourcePixel, Pixel> Converter>
	Bitmap(const Bitmap<SourcePixel>& source, Converter converter) : BaseBitmap(source.Width, source.Height, source.FaceCount, PixelFormat::GetFormat<Pixel>()), Pixels(new Pixel[size_t(source.Width) * source.Height * source.FaceCount])
	{
		for(size_t i = 0; i < source.PixelCount; i++)
		{
			Pixels[i] = converter(source.Pixels[i]);
		}
	}

	Bitmap(uint32_t width, uint32_t height, uint32_t faceCount = 1) : BaseBitmap(width, height, faceCount, PixelFormat::GetFormat<Pixel>()), Pixels(new Pixel[size_t(width) * height * faceCount]) {}

	Bitmap(const Bitmap<Pixel>& other) : Bitmap(other.Width, other.Height, other.FaceCount) 
	{
		other.CopyTo(*this);
	}

	Pixel* const Pixels;

	virtual       void* GetPixels()                { return Pixels; }
	virtual const void* GetPixels() const override { return Pixels; }

	      Pixel& GetPixel(uint32_t x, uint32_t y, uint32_t face = 0)       { return Pixels[PixelCount * face + (size_t(y) * Width + x)]; }
	const Pixel& GetPixel(uint32_t x, uint32_t y, uint32_t face = 0) const { return Pixels[PixelCount * face + (size_t(y) * Width + x)]; }

	      Pixel& GetPixel(const glm::uvec2& location, uint32_t face = 0)       { return GetPixel(location.x, location.y, face); }
	const Pixel& GetPixel(const glm::uvec2& location, uint32_t face = 0) const { return GetPixel(location.x, location.y, face); }

	void ZeroPixels()
	{
		memset(Pixels, 0, PixelCount * Format.Size);
	}

	void CopyTo(Bitmap<Pixel>& target) const
	{
		memcpy(target.Pixels, Pixels, PixelCount * sizeof(Pixel));
	}
};

template<typename Pixel>
BaseBitmapHandle PixelFormat::InternalCreateBitmap(uint32_t width, uint32_t height, uint32_t faceCount) { return std::make_shared<Bitmap<Pixel>>(width, height, faceCount); }

template<typename Pixel>
using BitmapHandle = std::shared_ptr<Bitmap<Pixel>>;

class Texture
{
public:
	Texture(uint32_t width, uint32_t height) : Width(width), Height(height) {}

	virtual ~Texture() {}

	union
	{
		struct
		{
			const uint32_t Width;
			const uint32_t Height;
		};
		const glm::uvec2 Size;
	};

	virtual void Bind(uint32_t slot) = 0;
};

class TextureAtlas
{
public:
	TextureAtlas(uint32_t cellWidth, uint32_t cellHeight, uint32_t columns, uint32_t rows) :
		CellWidth(cellWidth),
		CellHeight(cellHeight),
		Columns(columns),
		Rows(rows) {}

	virtual ~TextureAtlas() {}

	union
	{
		struct
		{
			const uint32_t CellWidth;
			const uint32_t CellHeight;
		};
		const glm::uvec2 Size;
	};

	const uint32_t Columns;
	const uint32_t Rows;

	virtual std::shared_ptr<Texture> GetTexture(size_t column, size_t row) = 0;
};

using TextureHandle = std::shared_ptr<Texture>;