#include "UIContext.hpp"

ContainerMouseState ContainerContext::GetMouseState() const
{
	glm::vec2 pixelPosition = Context.Mouse.GetPixelPosition();

	if(pixelPosition.x >= Position.x          && pixelPosition.y >= Position.y &&
	   pixelPosition.x <  Position.x + Size.x && pixelPosition.y <  Position.y + Size.y)
	{
		if(Context.Mouse.IsButtonDown(MouseButton::Left))
		{
			return ContainerMouseState::Pressed;
		}
		else
		{
			return ContainerMouseState::Hovered;
		}
	}
	
	return ContainerMouseState::None;
}

StackPanelContext ContainerContext::BeginStackPanel(Orientation orientation)
{
	return StackPanelContext(Context, *this, orientation);
}

ButtonContext ContainerContext::Button(const std::string& text)
{
	glm::vec2 pixelPosition = Context.Mouse.GetPixelPosition();

	ContainerMouseState mouseState = GetMouseState();

	if(Context.ShouldRender)
	{
		ButtonStyle& style = Context.CurrentTheme().TopStyle<ButtonStyle>();
		style.Render(Context.RenderContext, text, Position, Size, mouseState);
	}

	return ButtonContext(mouseState == ContainerMouseState::Hovered && Context.Mouse.GetButtonState(MouseButton::Left) == ButtonState::Released);
}

void ContainerContext::DragFloat(float& value, float minimum, float maximum, float step)
{
	ContainerMouseState mouseState = GetMouseState();

	if(Context.ShouldRender)
	{
		DragStyle& style = Context.CurrentTheme().TopStyle<DragStyle>();
		style.Render(Context.RenderContext, value, Position, Size, mouseState);
	}

	if(mouseState == ContainerMouseState::Pressed)
	{
		value += Context.Mouse.GetPixelDelta().x * step;
		value = glm::clamp(value, minimum, maximum);
	}
}

void ContainerContext::ProgressBar(Orientation orientation, float value, float minimum, float maximum)
{
	if(Context.ShouldRender)
	{
		ProgressBarStyle& style = Context.CurrentTheme().TopStyle<ProgressBarStyle>();
		style.Render(Context.RenderContext, orientation, value, minimum, maximum, Position, Size);
	}
}

//void ContainerContext::SliderFloat(float& value)
//{
//	bool isHovered = IsHovered();
//
//	if(Context.ShouldRender)
//	{
//		SliderStyle& style = Context.CurrentTheme().TopStyle<SliderStyle>();
//		style.Render(Context.RenderContext, value, Position, Size, isHovered, isHovered && Context.Mouse.IsButtonDown(MouseButton::Left));
//	}
//}

ContainerContext StackPanelContext::NextControl(float size)
{
	float currentPosition = m_currentPosition;
	m_currentPosition += size;
	if(ContentOrientation == Orientation::Horizontal)
	{
		return ContainerContext(Context, Parent.Position + glm::vec2(currentPosition, 0.0f), glm::vec2(size, Parent.Size.y));
	}
	else if(ContentOrientation == Orientation::Vertical)
	{
		return ContainerContext(Context, Parent.Position + glm::vec2(0.0f, currentPosition), glm::vec2(Parent.Size.x, size));
	}
}