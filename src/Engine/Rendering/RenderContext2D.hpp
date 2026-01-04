#pragma once

#include <unordered_map>
#include <glm/glm.hpp>
#include "Mesh.hpp"
#include "RenderDevice.hpp"
#include "RenderStream.hpp"

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

public:
	Font(TextureAtlasHandle textureAtlas, float fontSize, const std::vector<CharacterAttributes>& characters) :
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

	const TextureAtlasHandle TextureAtlas;

	float GetLargestHeight() const { return m_largestHeight; }

	const CharacterAttributes* Get(char character) const
	{
		const auto it = m_characterMap.find(character);
		if(it == m_characterMap.end())
		{
			return nullptr;
		}
		return &it->second;
	}

	glm::vec2 MeasureTextSize(std::string_view text, float fontSize) const
	{
		float resultWidth = 0.0f;

		const float fontScale = fontSize / FontSize;

		for(const char character : text)
		{
			const CharacterAttributes& attributes = *Get(character);
			resultWidth += attributes.CharacterAdvance * fontScale;
		}

		return { resultWidth, GetLargestHeight() * fontScale };
	}
private:
	std::unordered_map<char, CharacterAttributes> m_characterMap;
	float m_largestHeight;
};

using FontHandle = std::shared_ptr<Font>;

struct RectangleInfo
{
	RectangleInfo() = default;

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
};

struct EllipseInfo
{
	EllipseInfo() = default;

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

	CharacterInfo() = default;

	CharacterInfo(const glm::vec2& atlasOffset, const glm::vec2& offset, const glm::vec2& size, const glm::vec2& origin, float thickness, float borderThickness, const glm::vec4& borderColor, const glm::vec4& fillColor) :
		AtlasOffset(atlasOffset), 
		Offset(offset),
		Size(size),
		Origin(origin), 
		Thickness(thickness),
		BorderThickness(borderThickness), 
		BorderColor(borderColor),
		FillColor(fillColor) {}
};

enum class DrawingElementType
{
	Rectangle,
	Ellipse,
	Text,
};

struct ElementRange
{
	ElementRange(DrawingElementType element, size_t index, size_t count) : Element(element), Index(index), Count(count) {}

	const DrawingElementType Element;
	const size_t             Index;
	      size_t             Count;
};

struct FontInfo
{
	FontInfo(Font selectedFont, float scale) : SelectedFont(std::move(selectedFont)), Scale(scale) {}

	Font  SelectedFont;
	float Scale;
};

class RenderContext2D
{
public:
	RenderContext2D(RenderDevice& renderDevice, const RenderTargetHandle& renderTarget, const ShaderHandle& rectangleShader, const ShaderHandle& ellipseShader, const ShaderHandle& textShader, const MeshHandle& quad);

	RenderContext2D(const RenderContext2D&) = delete;

	RenderContext2D& operator=(const RenderContext2D&) = delete;

	[[nodiscard]] glm::vec2 GetBoundsSize() const { return m_renderDevice.ScreenBuffer->Size; }

	void DrawRectangle(const glm::vec2& position, const glm::vec2& scale, float borderThickness, float cornerRadius, const glm::vec4& borderColor, const glm::vec4& fillColor);
	void DrawEllipse(const glm::vec2& position, const glm::vec2& scale, float borderThickness, const glm::vec4& borderColor, const glm::vec4& fillColor);

	void DrawText(const glm::vec2& position, std::string_view text, const Font& font, float fontSize, float fontThickness, float borderThickness, const glm::vec4& borderColor, const glm::vec4& fillColor);

	void Flush();
private:
	RenderDevice& m_renderDevice;

	RenderTargetHandle m_renderTarget;

	ShaderHandle m_rectangleShader;
	ShaderHandle m_ellipseShader;
	ShaderHandle m_textShader;

	std::vector<RectangleInfo> m_rectangles;
	std::vector<EllipseInfo>   m_ellipses;
	std::vector<CharacterInfo> m_characters;

	RenderStream<RectangleInfo> m_rectangleStream;
	RenderStream<EllipseInfo>   m_ellipseStream;
	RenderStream<CharacterInfo> m_characterStream;

	std::vector<ElementRange> m_ranges;

	std::vector<FontInfo> m_fontQueue;

	void HandleRange(DrawingElementType shape, size_t index, size_t count = 1);
};