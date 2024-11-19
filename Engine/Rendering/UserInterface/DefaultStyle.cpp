#include "DefaultStyle.hpp"


static const glm::vec4 ButtonNormalBorderColor  = glm::vec4(1.0f, 1.0f, 1.0f, 0.2f);
static const glm::vec4 ButtonHoveredBorderColor = glm::vec4(1.0f, 0.5f, 0.0f, 0.4f);
static const glm::vec4 ButtonPressedBorderColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.1f);

static const glm::vec4 ButtonNormalFaceColor  = glm::vec4(1.0f, 1.0f, 1.0f, 0.1f);
static const glm::vec4 ButtonHoveredFaceColor = glm::vec4(1.0f, 0.5f, 0.0f, 0.2f);
static const glm::vec4 ButtonPressedFaceColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);

void DefaultButtonStyle::Render(RenderContext2D& renderContext, std::string_view text, const glm::vec2& position, const glm::vec2& size, ContainerMouseState mouseState)
{
    glm::vec4 borderColor = ButtonNormalBorderColor;
    glm::vec4 faceColor   = ButtonNormalFaceColor;
    if(mouseState == ContainerMouseState::Hovered)
    {
        borderColor = ButtonHoveredBorderColor;
        faceColor = ButtonHoveredFaceColor;
    }
    else if(mouseState == ContainerMouseState::Pressed)
    {
        borderColor = ButtonPressedBorderColor;
        faceColor = ButtonPressedFaceColor;
    }

    renderContext.DrawRectangle(position + 5.0f, size - 10.0f, 3.0f, 5.0f, borderColor, faceColor);
    glm::vec2 textSize = renderContext.MeasureTextSize(text, TextFontSize, TextFont);
    renderContext.DrawText(position + size * 0.5f - textSize * 0.5f, text, TextFont, TextFontSize, 0.5f, 0.0f, glm::vec4(), glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));
}

void DefaultDragStyle::Render(RenderContext2D& renderContext, float value, const glm::vec2& position, const glm::vec2& size, ContainerMouseState mouseState)
{
    std::string stringValue = std::to_string(value);
    glm::vec2 textSize = renderContext.MeasureTextSize(stringValue, TextFontSize, TextFont);
    renderContext.DrawText(position + size * 0.5f - textSize * 0.5f, stringValue, TextFont, TextFontSize, 0.5f, 0.0f, glm::vec4(), glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));
}

void DefaultProgressBarStyle::Render(RenderContext2D& renderContext, Orientation orientation, float value, float minimum, float maximum, const glm::vec2& position, const glm::vec2& size)
{
    float fraction = (value - minimum) / (maximum - minimum);

    glm::vec2 innerBarOffset = position + 10.0f;
    glm::vec2 innerBarSize = size - 20.0f;

    if(orientation == Orientation::Horizontal)
    {
        innerBarSize.x *= fraction;
    }
    else if(orientation == Orientation::Vertical)
    {
        float totalHeight = innerBarSize.y;
        innerBarSize.y *= fraction;
        innerBarOffset.y += totalHeight - innerBarSize.y;
    }

    renderContext.DrawRectangle(position + 5.0f, size - 10.0f, 3.0f, 5.0f, ButtonNormalBorderColor, glm::vec4(0.0f));
    renderContext.DrawRectangle(innerBarOffset, innerBarSize, 0.0f, 3.0f, glm::vec4(0.0f), ButtonHoveredFaceColor);
    
    m_outputStream << value;
    m_outputStream.flush();

    glm::vec2 textSize = renderContext.MeasureTextSize(m_outputStream.view(), TextFontSize, TextFont);
    renderContext.DrawText(position + size * 0.5f - textSize * 0.5f, m_outputStream.view(), TextFont, TextFontSize, 0.5f, 0.0f, glm::vec4(), glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));

    m_outputStream.str("");
}
