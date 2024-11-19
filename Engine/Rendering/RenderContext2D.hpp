#pragma once

#include <glm/glm.hpp>
#include "Mesh.hpp"
#include "RenderDevice.hpp"

struct CharacterAttributes
{
	CharacterAttributes(char character, const glm::vec2 position, const glm::vec2& size, const glm::vec2& origin, float characterAdvance) :
		Character(character), Position(position), Size(size), Origin(origin), CharacterAdvance(characterAdvance) {}

	char      Character;
	glm::vec2 Position;
	glm::vec2 Size;
	glm::vec2 Origin;
	float     CharacterAdvance;
};

class Font
{
private:
	std::unordered_map<char, CharacterAttributes> m_characterMap;
	float m_largestHeight;
public:
	Font(TextureHandle textureAtlas, float fontSize, const std::vector<CharacterAttributes>& characters) :
		m_largestHeight(0),
		FontSize(fontSize),
		TextureAtlas(textureAtlas) 
	{
		for(const CharacterAttributes& attributes : characters)
		{
			m_characterMap.insert(std::make_pair(attributes.Character, attributes));
			m_largestHeight = glm::max(m_largestHeight, attributes.Size.y);
		}
	}

	const float FontSize;

	const TextureHandle TextureAtlas;

	float GetLargestHeight() const { return m_largestHeight; }

	const CharacterAttributes* Get(char character) const
	{
		auto it = m_characterMap.find(character);
		if(it == m_characterMap.end())
		{
			return nullptr;
		}
		return &it->second;
	}
};

using FontHandle = std::shared_ptr<Font>;

struct RectangleInfo
{
	RectangleInfo(const glm::vec2& position, const glm::vec2& scale, float borderThickness, float cornerRadius, const glm::vec4& fillColor, const glm::vec4& borderColor) :
		Position(position), 
		Scale(scale),
		BorderThickness(borderThickness),
		CornerRadius(cornerRadius), 
		FillColor(fillColor),
		BorderColor(borderColor) {}

	glm::vec2 Position;
	glm::vec2 Scale;
	float     BorderThickness;
	float     CornerRadius;
	glm::vec4 FillColor;
	glm::vec4 BorderColor;

	inline static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(Position)>();
		result.AddAttribute<decltype(Scale)>();
		result.AddAttribute<decltype(BorderThickness)>();
		result.AddAttribute<decltype(CornerRadius)>();
		result.AddAttribute<decltype(BorderColor)>();
		result.AddAttribute<decltype(FillColor)>();
		return result;
	}
};

struct EllipseInfo
{
	EllipseInfo(const glm::vec2& position, const glm::vec2& scale, float borderThickness, const glm::vec4& fillColor, const glm::vec4& borderColor) :
		Position(position),
		Scale(scale),
		BorderThickness(borderThickness),
		FillColor(fillColor),
		BorderColor(borderColor) {}

	glm::vec2 Position;
	glm::vec2 Scale;
	float     BorderThickness;
	glm::vec4 FillColor;
	glm::vec4 BorderColor;

	inline static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(Position)>();
		result.AddAttribute<decltype(Scale)>();
		result.AddAttribute<decltype(BorderThickness)>();
		result.AddAttribute<decltype(BorderColor)>();
		result.AddAttribute<decltype(FillColor)>();
		return result;
	}
};

struct CharacterInfo
{
	glm::vec2 AtlasOffset;
	glm::vec2 Offset;
	glm::vec2 Size;
	glm::vec2 Origin;
	float     Thickness;
	float     BorderThickness;
	glm::vec4 BorderColor;
	glm::vec4 FillColor;

	CharacterInfo(const glm::vec2& atlasOffset, const glm::vec2& offset, const glm::vec2& size, const glm::vec2& origin, float thickness, float borderThickness, const glm::vec4& borderColor, const glm::vec4& fillColor) :
		AtlasOffset(atlasOffset), 
		Offset(offset),
		Size(size),
		Origin(origin), 
		Thickness(thickness),
		BorderThickness(borderThickness), 
		BorderColor(borderColor),
		FillColor(fillColor) {}

	inline static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(AtlasOffset)>();
		result.AddAttribute<decltype(Offset)>();
		result.AddAttribute<decltype(Size)>();
		result.AddAttribute<decltype(Origin)>();
		result.AddAttribute<decltype(Thickness)>();
		result.AddAttribute<decltype(BorderThickness)>();
		result.AddAttribute<decltype(BorderColor)>();
		result.AddAttribute<decltype(FillColor)>();
		return result;
	}
};

enum class DrawingElement
{
	Rectangle,
	Ellipse,
	Text,
};

struct ElementRange
{
	ElementRange(DrawingElement element, size_t index, size_t count) : Element(element), Index(index), Count(count) {}

	const DrawingElement Element;
	const size_t         Index;
	      size_t         Count;
};

struct FontInfo
{
public:
	FontInfo(FontHandle font, float scale) : Font(font), Scale(scale) {}

	FontHandle Font;
	float      Scale;
};

class RenderContext2D
{
private:
	RenderDevice& m_renderDevice;

	VertexArrayHandle m_rectangleQuad;
	VertexArrayHandle m_ellipseQuad;
	VertexArrayHandle m_textQuad;

	ShaderHandle m_rectangleShader;
	ShaderHandle m_ellipseShader;
	ShaderHandle m_textShader;

	Array<RectangleInfo> m_rectangleQueue;
	Array<EllipseInfo>   m_ellipseQueue;
	Array<CharacterInfo> m_characterQueue;

	std::vector<ElementRange> m_ranges;

	std::vector<FontInfo> m_fontQueue;

	void HandleRange(DrawingElement shape, BaseArraySlice queue, size_t count = 1);
public:
	RenderContext2D(RenderDevice& renderDevice, ShaderHandle rectangleShader, ShaderHandle ellipseShader, ShaderHandle textShader, MeshHandle quad);

	RenderContext2D(const RenderContext2D&) = delete;

	RenderContext2D& operator=(const RenderContext2D&) = delete;

	glm::vec2 GetBoundsSize() const { return m_renderDevice.ScreenBuffer->Size; }

	glm::vec2 MeasureTextSize(std::string_view text, float fontSize, FontHandle font) const;

	void DrawRectangle(const glm::vec2& position, const glm::vec2& scale, float borderThickness, float cornerRadius, const glm::vec4& borderColor, const glm::vec4& fillColor);
	void DrawEllipse(const glm::vec2& position, const glm::vec2& scale, float borderThickness, const glm::vec4& borderColor, const glm::vec4& fillColor);

	void DrawText(const glm::vec2& position, std::string_view text, FontHandle font, float fontSize, float fontThickness, float borderThickness, const glm::vec4& borderColor, const glm::vec4& fillColor);

	void Flush();
};