#include "RenderContext2D.hpp"

void RenderContext2D::HandleRange(DrawingElement element, BaseArraySlice queue, size_t count)
{
	if(m_ranges.size() == 0)
	{
		m_ranges.emplace_back(element, queue.Count(), count);
		return;
	}

	ElementRange* range = &m_ranges[m_ranges.size() - 1];
	if(range->Element != element)
	{
		range = &m_ranges.emplace_back(element, queue.Count(), count);
	}
	else
	{
		range->Count += count;
	}
}

RenderContext2D::RenderContext2D(RenderDevice& renderDevice, ShaderHandle rectangleShader, ShaderHandle ellipseShader, ShaderHandle textShader, MeshHandle quad) :
	m_renderDevice(renderDevice),
	m_rectangleQuad(quad->CreateVertexArray(RectangleInfo::GetLayout())),
	m_ellipseQuad(quad->CreateVertexArray(EllipseInfo::GetLayout())),
	m_textQuad(quad->CreateVertexArray(CharacterInfo::GetLayout())),
	m_rectangleShader(rectangleShader),
	m_ellipseShader(ellipseShader),
	m_textShader(textShader)
{
}

void RenderContext2D::DrawRectangle(const glm::vec2& position, const glm::vec2& scale, float borderThickness, float cornerRadius, const glm::vec4& borderColor, const glm::vec4& fillColor)
{
	HandleRange(DrawingElement::Rectangle, m_rectangleQueue);
	m_rectangleQueue.Emplace(position, scale, borderThickness, cornerRadius, borderColor, fillColor);
}

void RenderContext2D::DrawEllipse(const glm::vec2& position, const glm::vec2& scale, float borderThickness, const glm::vec4& borderColor, const glm::vec4& fillColor)
{
	HandleRange(DrawingElement::Ellipse, m_ellipseQueue);
	m_ellipseQueue.Emplace(position, scale, borderThickness, borderColor, fillColor);
}

void RenderContext2D::DrawText(const glm::vec2& position, std::string_view text, FontHandle font, float fontSize, float fontThickness, float borderThickness, const glm::vec4& borderColor, const glm::vec4& fillColor)
{
	m_ranges.emplace_back(DrawingElement::Text, m_characterQueue.Count(), text.length());
	m_fontQueue.emplace_back(font, fontSize / font->FontSize);
	float cursorOffset = 0.0f;
	const float fontScale = fontSize / font->FontSize;
	for(char character : text)
	{
		const CharacterAttributes& attributes = *font->Get(character);
		const float characterAdvance = attributes.CharacterAdvance * fontScale * 0.5f;
		cursorOffset += characterAdvance;
		m_characterQueue.Emplace(attributes.Position, position + glm::vec2(cursorOffset, 0.0f), attributes.Size, attributes.Origin - glm::vec2(0, font->GetLargestHeight() * 1.5f), fontThickness, borderThickness, borderColor, fillColor);
		cursorOffset += characterAdvance;
	}
}


glm::vec2 RenderContext2D::MeasureTextSize(std::string_view text, float fontSize, FontHandle font) const
{
	float resultWidth = 0.0f;

	const float fontScale = fontSize / font->FontSize;

	for(char character : text)
	{
		const CharacterAttributes& attributes = *font->Get(character);
		resultWidth += attributes.CharacterAdvance * fontScale;
	}

	return glm::vec2(resultWidth, font->GetLargestHeight() * fontScale);
}

void RenderContext2D::Flush()
{
	m_renderDevice.Disable(DepthTest);
	m_renderDevice.Enable(Blending);
	m_renderDevice.SetBlendFunction(BlendFactor::SourceAlpha, BlendFactor::OneMinusSourceAlpha);
	
	size_t fontIndex = 0;
	for(const ElementRange& range : m_ranges)
	{
		if(range.Element == DrawingElement::Rectangle)
		{
			m_rectangleShader->Get<glm::vec2>("u_screenSize") = m_renderDevice.ScreenBuffer->Size;
			m_rectangleShader->Use(nullptr);
			ArraySlice<RectangleInfo> slice = m_rectangleQueue;
			m_rectangleQuad->Draw(slice.Slice(range.Index, range.Count));
		}
		else if(range.Element == DrawingElement::Ellipse)
		{
			m_ellipseShader->Get<glm::vec2>("u_screenSize") = m_renderDevice.ScreenBuffer->Size;
			m_ellipseShader->Use(nullptr);
			ArraySlice<EllipseInfo> slice = m_ellipseQueue;
			m_ellipseQuad->Draw(slice.Slice(range.Index, range.Count));
		}
		else if(range.Element == DrawingElement::Text)
		{
			m_textShader->Get<float>("u_scale") = m_fontQueue[fontIndex].Scale;

			m_textShader->Get<glm::vec2>("u_screenSize") = m_renderDevice.ScreenBuffer->Size;
			m_textShader->Get<glm::vec2>("u_atlasSize") = m_fontQueue[fontIndex].Font->TextureAtlas->Size;
			m_textShader->SetTexture("u_textureAtlas", m_fontQueue[fontIndex].Font->TextureAtlas);
			
			m_textShader->Use(nullptr);
			ArraySlice<CharacterInfo> slice = m_characterQueue;
			m_textQuad->Draw(slice.Slice(range.Index, range.Count));
			++fontIndex;
		}
	}

	m_rectangleQueue.Clear();
	m_ellipseQueue.Clear();
	m_characterQueue.Clear();
	m_ranges.clear();
	m_fontQueue.clear();

	m_renderDevice.Disable(Blending);
}
