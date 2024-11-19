#pragma once

#include <sstream>

#include "UIContext.hpp"

struct DefaultButtonStyle : public ButtonStyle
{
	DefaultButtonStyle(FontHandle textFont, float textFontSize) : TextFont(textFont), TextFontSize(textFontSize) {}

	float      TextFontSize;
	FontHandle TextFont;

	void Render(RenderContext2D& renderContext, std::string_view text, const glm::vec2& position, const glm::vec2& size, ContainerMouseState mouseState) override;
};

struct DefaultDragStyle : public DragStyle
{
	DefaultDragStyle(FontHandle textFont, float textFontSize) : TextFont(textFont), TextFontSize(textFontSize) {}

	float      TextFontSize;
	FontHandle TextFont;

	void Render(RenderContext2D& renderContext, float value, const glm::vec2& position, const glm::vec2& size, ContainerMouseState mouseState) override;
};

struct DefaultProgressBarStyle : public ProgressBarStyle
{
	std::ostringstream m_outputStream;

	DefaultProgressBarStyle(FontHandle textFont, float textFontSize) : TextFont(textFont), TextFontSize(textFontSize) {}

	DefaultProgressBarStyle(const DefaultProgressBarStyle& other) : TextFontSize(other.TextFontSize), TextFont(other.TextFont) {}

	float      TextFontSize;
	FontHandle TextFont;

	void Render(RenderContext2D& renderContext, Orientation orientation, float value, float minimum, float maximum, const glm::vec2& position, const glm::vec2& size) override;
};