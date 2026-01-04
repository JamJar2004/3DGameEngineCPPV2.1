#pragma once

#include <stack>

#include "../../Core/Input.hpp"
#include "../RenderContext2D.hpp"

enum class Orientation
{
	Horizontal,
	Vertical,
};

namespace std
{
	template<typename T>
	concept virtual_destructible = has_virtual_destructor_v<T>;

	template<typename T>
	concept numeric = is_arithmetic_v<T>;
}

class ArenaStack
{
private:
	std::vector<uint8_t>   m_data;
	std::vector<TypeInfo*> m_types;
public:
	ArenaStack() = default;

	ArenaStack(const ArenaStack& other) : m_data(other.m_data), m_types(other.m_types)
	{
		size_t offset = 0;
		for(TypeInfo* type : other.m_types)
		{
			const void* source = other.m_data.data() + offset;
			      void* target =       m_data.data() + offset;

			type->CopyConstructor(source, target);
			offset += type->Size;
		}
	}

	~ArenaStack()
	{
		size_t offset = 0;
		for(TypeInfo* type : m_types)
		{
			void* element = m_data.data() + offset;
			type->Destructor(element);
			offset += type->Size;
		}
	}

	ArenaStack& operator=(const ArenaStack&) = delete;

	template<typename T, typename... Args>
	T& Push(Args&&... args) requires std::constructible_from<T, Args...>
	{
		TypeInfo* type = TypeInfo::Get<T>();
		size_t startIndex = m_data.size();
		m_types.push_back(type);
		m_data.resize(startIndex + type->Size);
		void* address = m_data.data() + startIndex;
		new(address) T(args...);
		return *(T*)address;
	}
	
	void Pop()
	{
		TypeInfo* lastElementType = m_types[m_types.size() - 1];
		m_types.pop_back();
		void* address = m_data.data() - lastElementType->Size;
		lastElementType->Destructor(address);
		m_data.resize(m_data.size() - lastElementType->Size);
	}
	
	void* Top()
	{
		size_t lastElementSize = m_types[m_types.size() - 1]->Size;
		return m_data.data() + (m_data.size() - lastElementSize);
	}

	const void* Top() const
	{
		size_t lastElementSize = m_types[m_types.size() - 1]->Size;
		return m_data.data() + (m_data.size() - lastElementSize);
	}
};

class Theme
{
private:
	std::unordered_map<TypeInfo*, ArenaStack> m_styleStacks;
public:
	template<std::virtual_destructible BaseStyle, std::derived_from<BaseStyle> Style, typename... Args>
	Style& PushStyle(Args&&... args) requires std::constructible_from<Style, Args...>
	{
		ArenaStack& stack = m_styleStacks[TypeInfo::Get<BaseStyle>()];
		return stack.Push<Style>(args...);
	}

	template<std::virtual_destructible BaseStyle>
	BaseStyle& TopStyle()
	{
		ArenaStack& stack = m_styleStacks[TypeInfo::Get<BaseStyle>()];
		return *(BaseStyle*)stack.Top();
	}

	template<std::virtual_destructible BaseStyle>
	void PopStyle()
	{
		ArenaStack& stack = m_styleStacks[TypeInfo::Get<BaseStyle>()];
		stack.Pop();
	}
};

enum class ContainerMouseState
{
	None,
	Hovered,
	Pressed,
};

class ButtonContext
{
public:
	ButtonContext(bool isClicked) : IsClicked(isClicked) {}

	const bool IsClicked;
};

struct ButtonStyle
{
	virtual ~ButtonStyle() {}

	virtual void Render(RenderContext2D& renderContext, std::string_view text, const glm::vec2& position, const glm::vec2& size, ContainerMouseState mouseState) = 0;
};

struct DragStyle
{
	virtual ~DragStyle() {}

	virtual void Render(RenderContext2D& renderContext, float value, const glm::vec2& position, const glm::vec2& size, ContainerMouseState mouseState) = 0;
};

struct ProgressBarStyle
{
	virtual ~ProgressBarStyle() {}

	virtual void Render(RenderContext2D& renderContext, Orientation orientation, float value, float minimum, float maximum, const glm::vec2& position, const glm::vec2& size) = 0;
};

struct SliderStyle
{
	virtual ~SliderStyle() {}

	virtual void Render(RenderContext2D& renderContext, float value, const glm::vec2& position, const glm::vec2& size, ContainerMouseState mouseState) = 0;
};




class UIContext;
class StackPanelContext;


class ContainerContext
{
public:
	ContainerContext(UIContext& context, const glm::vec2& position, const glm::vec2& size) :
		Context(context), Position(position), Size(size) {}

	UIContext& Context;

	const glm::vec2 Position;
	const glm::vec2 Size;

	ContainerMouseState GetMouseState() const;

	glm::vec2 Centre() const { return Position + Size * 0.5f; }

	StackPanelContext BeginStackPanel(Orientation orientation);

	ButtonContext Button(const std::string& text);
	
	void DragFloat(float& value, float minimum = std::numeric_limits<float>().min(), float maximum = std::numeric_limits<float>().max(), float step = 1.0f);

	void ProgressBar(Orientation orientation, float value, float minimum = std::numeric_limits<float>().min(), float maximum = std::numeric_limits<float>().max());

	//void SliderFloat(float& value);
};

class UIContext
{
private:
	std::stack<Theme> m_themes;
public:
	UIContext(KeyboardDevice& keyboard, MouseDevice& mouse, RenderContext2D& renderContext, const Theme& theme) :
		Keyboard(keyboard), 
		Mouse(mouse), 
		RenderContext(renderContext),
		RootContainer(*this, glm::vec2(0, 0), renderContext.GetBoundsSize()),
		ShouldRender(false)
	{
		m_themes.push(theme);
	}

	UIContext(const UIContext&) = delete;

	UIContext& operator=(const UIContext&) = delete;

	KeyboardDevice&  Keyboard;
	MouseDevice&     Mouse;
	RenderContext2D& RenderContext;

	ContainerContext RootContainer;

	bool ShouldRender;

	glm::vec2 GetBoundsSize() const { return RenderContext.GetBoundsSize(); }

	Theme& PushTheme() { return m_themes.emplace(); }

	Theme& CurrentTheme() { return m_themes.top(); }

	void PopTheme() { m_themes.pop(); }
};

class StackPanelContext
{
private:
	float m_currentPosition;
public:
	StackPanelContext(UIContext& context, ContainerContext& parent, Orientation orientation) : Context(context), Parent(parent), m_currentPosition(0.0f), ContentOrientation(orientation) {}

	UIContext& Context;

	ContainerContext& Parent;

	const Orientation ContentOrientation;

	ContainerContext NextControl(float size);
};

class GridPanelContext
{
private:
public:
	const size_t Rows;
	const size_t Columns;
};
