#include "RenderContext2D.hpp"

#include "Engine/EngineSystems/AnimationSystem.hpp"

void RenderContext2D::HandleRange(DrawingElementType shape, size_t index, size_t count)
{
	if(m_ranges.size() == 0)
	{
		m_ranges.emplace_back(shape, index, count);
	}
	else if(ElementRange* range = &m_ranges[m_ranges.size() - 1]; range->Element != shape)
	{
		m_ranges.emplace_back(shape, index, count);
	}
	else
	{
		range->Count += count;
	}
}

RenderContext2D::RenderContext2D(RenderDevice& renderDevice, const RenderTargetHandle& renderTarget, const ShaderHandle& rectangleShader, const ShaderHandle& ellipseShader, const ShaderHandle& textShader, const MeshHandle& quad) :
	m_renderDevice(renderDevice),
	m_renderTarget(renderTarget),
	m_rectangleShader(rectangleShader),
	m_ellipseShader(ellipseShader),
	m_textShader(textShader),
	m_rectangleStream(renderDevice.CreateRenderBuffer<RectangleInfo>(5000, RectangleInfo()), quad, rectangleShader, renderTarget),
	m_ellipseStream  (renderDevice.CreateRenderBuffer<  EllipseInfo>(5000,   EllipseInfo()), quad, ellipseShader, renderTarget),
	m_characterStream(renderDevice.CreateRenderBuffer<CharacterInfo>(5000, CharacterInfo()), quad, textShader, renderTarget)
{
}

void RenderContext2D::DrawRectangle(const glm::vec2& position, const glm::vec2& scale, float borderThickness, float cornerRadius, const glm::vec4& borderColor, const glm::vec4& fillColor)
{
	HandleRange(DrawingElementType::Rectangle, m_rectangles.size());
	m_rectangles.emplace_back(position, scale, borderThickness, cornerRadius, fillColor, borderColor);
}

void RenderContext2D::DrawEllipse(const glm::vec2& position, const glm::vec2& scale, float borderThickness, const glm::vec4& borderColor, const glm::vec4& fillColor)
{
	HandleRange(DrawingElementType::Ellipse, m_ellipses.size());
	m_ellipses.emplace_back(position, scale, borderThickness, fillColor, borderColor);
}

void RenderContext2D::DrawText(const glm::vec2& position, std::string_view text, const Font& font, float fontSize, float fontThickness, float borderThickness, const glm::vec4& borderColor, const glm::vec4& fillColor)
{
	m_ranges.emplace_back(DrawingElementType::Text, m_characters.size(), text.length());
	m_fontQueue.emplace_back(font, fontSize / font.FontSize);
	float cursorOffset = 0.0f;
	const float fontScale = fontSize / font.FontSize;
	for(const char character : text)
	{
		const CharacterAttributes& attributes = *font.Get(character);
		const float characterAdvance = attributes.CharacterAdvance * fontScale * 0.5f;
		cursorOffset += characterAdvance;
		m_characters.emplace_back(attributes.Position, position + glm::vec2(cursorOffset, 0.0f), attributes.Size, attributes.Origin - glm::vec2(0, font.GetLargestHeight() * 1.5f), fontThickness, borderThickness, borderColor, fillColor);
		cursorOffset += characterAdvance;
	}
}


void RenderContext2D::Flush()
{
	m_renderDevice.Disable(DepthTest);
	m_renderDevice.Enable(Blending);
	m_renderDevice.SetBlendFunction(BlendFactor::SourceAlpha, BlendFactor::OneMinusSourceAlpha);
	
	size_t fontIndex = 0;
	for(const ElementRange& range : m_ranges)
	{
		if(range.Element == DrawingElementType::Rectangle)
		{
			//m_rectangleShader->Get<glm::vec2>("u_screenSize") = m_renderDevice.ScreenBuffer->Size;
			//m_rectangleShader->Use(nullptr);
			m_rectangleStream.WriteRange(ConstBufferSlice(m_rectangles.data() + range.Index, range.Count));
			m_rectangleStream.Flush();
		}
		else if(range.Element == DrawingElementType::Ellipse)
		{
			//m_ellipseShader->Get<glm::vec2>("u_screenSize") = m_renderDevice.ScreenBuffer->Size;
			//m_ellipseShader->Use(nullptr);
			m_ellipseStream.WriteRange(ConstBufferSlice(m_ellipses.data() + range.Index, range.Count));
			m_ellipseStream.Flush();
		}
		else if(range.Element == DrawingElementType::Text)
		{
			//m_textShader->Get<float>("u_scale") = m_fontQueue[fontIndex].Scale;

			//m_textShader->Get<glm::vec2>("u_screenSize") = m_renderDevice.ScreenBuffer->Size;
			//m_textShader->Get<glm::vec2>("u_atlasSize") = m_fontQueue[fontIndex].Font.TextureAtlas->Size;
			//m_textShader->SetTexture("u_textureAtlas", m_fontQueue[fontIndex].Font.TextureAtlas);
			
			//m_textShader->Use(nullptr);
			m_characterStream.WriteRange(ConstBufferSlice(m_characters.data() + range.Index, range.Count));
			m_characterStream.Flush();
			++fontIndex;
		}
	}

	m_rectangles.clear();
	m_ellipses.clear();
	m_characters.clear();
	m_ranges.clear();
	m_fontQueue.clear();

	m_renderDevice.Disable(Blending);
}
