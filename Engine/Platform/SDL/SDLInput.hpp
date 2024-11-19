#pragma once

#include <SDL2/SDL.h>
#include <unordered_map>

#include "../../Core/Input.hpp"

class SDLKeyboardDevice : public KeyboardDevice
{
private:
	std::unordered_map<SDL_Scancode, Key> m_keys;
public:
	SDLKeyboardDevice()
	{
		m_keys[SDL_SCANCODE_0] = Key::Num_0;
		m_keys[SDL_SCANCODE_1] = Key::Num_1;
		m_keys[SDL_SCANCODE_2] = Key::Num_2;
		m_keys[SDL_SCANCODE_3] = Key::Num_3;
		m_keys[SDL_SCANCODE_4] = Key::Num_4;
		m_keys[SDL_SCANCODE_5] = Key::Num_5;
		m_keys[SDL_SCANCODE_6] = Key::Num_6;
		m_keys[SDL_SCANCODE_7] = Key::Num_7;
		m_keys[SDL_SCANCODE_8] = Key::Num_8;
		m_keys[SDL_SCANCODE_9] = Key::Num_9;

		m_keys[SDL_SCANCODE_A] = Key::A;
		m_keys[SDL_SCANCODE_B] = Key::B;
		m_keys[SDL_SCANCODE_C] = Key::C;
		m_keys[SDL_SCANCODE_D] = Key::D;
		m_keys[SDL_SCANCODE_E] = Key::E;
		m_keys[SDL_SCANCODE_F] = Key::F;
		m_keys[SDL_SCANCODE_G] = Key::G;
		m_keys[SDL_SCANCODE_H] = Key::H;
		m_keys[SDL_SCANCODE_I] = Key::I;
		m_keys[SDL_SCANCODE_J] = Key::J;
		m_keys[SDL_SCANCODE_K] = Key::K;
		m_keys[SDL_SCANCODE_L] = Key::L;
		m_keys[SDL_SCANCODE_M] = Key::M;
		m_keys[SDL_SCANCODE_N] = Key::N;
		m_keys[SDL_SCANCODE_O] = Key::O;
		m_keys[SDL_SCANCODE_P] = Key::P;
		m_keys[SDL_SCANCODE_Q] = Key::Q;
		m_keys[SDL_SCANCODE_R] = Key::R;
		m_keys[SDL_SCANCODE_S] = Key::S;
		m_keys[SDL_SCANCODE_T] = Key::T;
		m_keys[SDL_SCANCODE_U] = Key::U;
		m_keys[SDL_SCANCODE_V] = Key::V;
		m_keys[SDL_SCANCODE_W] = Key::W;
		m_keys[SDL_SCANCODE_X] = Key::X;
		m_keys[SDL_SCANCODE_Y] = Key::Y;
		m_keys[SDL_SCANCODE_Z] = Key::Z;

		m_keys[SDL_SCANCODE_F1 ] = Key::F1;
		m_keys[SDL_SCANCODE_F2 ] = Key::F2;
		m_keys[SDL_SCANCODE_F3 ] = Key::F3;
		m_keys[SDL_SCANCODE_F4 ] = Key::F4;
		m_keys[SDL_SCANCODE_F5 ] = Key::F5;
		m_keys[SDL_SCANCODE_F6 ] = Key::F6;
		m_keys[SDL_SCANCODE_F7 ] = Key::F7;
		m_keys[SDL_SCANCODE_F8 ] = Key::F8;
		m_keys[SDL_SCANCODE_F9 ] = Key::F9;
		m_keys[SDL_SCANCODE_F10] = Key::F10;
		m_keys[SDL_SCANCODE_F11] = Key::F11;
		m_keys[SDL_SCANCODE_F12] = Key::F12;

		m_keys[SDL_SCANCODE_COMMA       ] = Key::Comma;
		m_keys[SDL_SCANCODE_PERIOD      ] = Key::Dot;
		m_keys[SDL_SCANCODE_SLASH       ] = Key::ForwardSlash;
		m_keys[SDL_SCANCODE_SEMICOLON   ] = Key::SemiColon;
		m_keys[SDL_SCANCODE_APOSTROPHE  ] = Key::Apostrophe;
		m_keys[SDL_SCANCODE_LEFTBRACKET ] = Key::LeftBracket;
		m_keys[SDL_SCANCODE_RIGHTBRACKET] = Key::RightBracket;
		m_keys[SDL_SCANCODE_BACKSLASH   ] = Key::BackwardSlash;
		m_keys[SDL_SCANCODE_MINUS       ] = Key::Dash;
		m_keys[SDL_SCANCODE_EQUALS      ] = Key::Equals;

		m_keys[SDL_SCANCODE_ESCAPE   ] = Key::Escape;
		m_keys[SDL_SCANCODE_LCTRL    ] = Key::LeftControl;
		m_keys[SDL_SCANCODE_RCTRL    ] = Key::RightControl;
		m_keys[SDL_SCANCODE_LSHIFT   ] = Key::LeftShift;
		m_keys[SDL_SCANCODE_RSHIFT   ] = Key::RightShift;
		m_keys[SDL_SCANCODE_LALT     ] = Key::LeftAlt;
		m_keys[SDL_SCANCODE_RALT     ] = Key::RightAlt;
		m_keys[SDL_SCANCODE_TAB      ] = Key::Tab;
		m_keys[SDL_SCANCODE_LGUI     ] = Key::LeftSuper;
		m_keys[SDL_SCANCODE_RGUI     ] = Key::RightSuper;
		m_keys[SDL_SCANCODE_MENU     ] = Key::Menu;
		m_keys[SDL_SCANCODE_SPACE    ] = Key::Space;
		m_keys[SDL_SCANCODE_RETURN   ] = Key::Enter;
		m_keys[SDL_SCANCODE_INSERT   ] = Key::Insert;
		m_keys[SDL_SCANCODE_BACKSPACE] = Key::Backspace;
		m_keys[SDL_SCANCODE_DELETE   ] = Key::Delete;
		m_keys[SDL_SCANCODE_HOME     ] = Key::Home;
		m_keys[SDL_SCANCODE_END      ] = Key::End;
		m_keys[SDL_SCANCODE_PAGEUP   ] = Key::PageUp;
		m_keys[SDL_SCANCODE_PAGEDOWN ] = Key::PageDown;

		m_keys[SDL_SCANCODE_UP   ] = Key::Up;
		m_keys[SDL_SCANCODE_DOWN ] = Key::Down;
		m_keys[SDL_SCANCODE_LEFT ] = Key::Left;
		m_keys[SDL_SCANCODE_RIGHT] = Key::Right;
	}

	void OnEvent(const SDL_Event& e)
	{
		switch(e.type)
		{
			case SDL_KEYDOWN:
				if(!e.key.repeat)
					TriggerKeyDown(m_keys[e.key.keysym.scancode]);

				break;
			case SDL_KEYUP:
				if(!e.key.repeat)
					TriggerKeyUp(m_keys[e.key.keysym.scancode]);

				break;
		}
	}
};

class SDLMouseDevice : public MouseDevice
{
private:
	std::unordered_map<uint8_t, MouseButton> m_buttons;

	glm::vec2 m_dimensions;

	glm::ivec2 m_pixelPosition;

	SDL_Window* m_window;
public:
	SDLMouseDevice(GraphicsMode graphicsMode, SDL_Window* window) : 
		m_dimensions(graphicsMode.Width, graphicsMode.Height), m_window(window)
	{
		m_buttons[SDL_BUTTON_LEFT  ] = MouseButton::Left;
		m_buttons[SDL_BUTTON_MIDDLE] = MouseButton::Middle;
		m_buttons[SDL_BUTTON_RIGHT ] = MouseButton::Right;
	}

	void OnEvent(const SDL_Event& e)
	{
		switch(e.type)
		{
			case SDL_MOUSEBUTTONDOWN:
				TriggerButtonDown(m_buttons[e.button.button]);
				break;
			case SDL_MOUSEBUTTONUP:
				TriggerButtonUp(m_buttons[e.button.button]);
				break;
			case SDL_MOUSEMOTION:
				TriggerMove(((glm::vec2(e.motion.x, e.motion.y) / m_dimensions) * 2.0f - 1.0f) * glm::vec2(1.0f, -1.0f), glm::ivec2(e.motion.x, e.motion.y));
				break;
		}
	}

	virtual void SetPosition(const glm::vec2& position) override
	{
		int x = int(( position.x * 0.5 + 0.5) * m_dimensions.x);
		int y = int((-position.y * 0.5 + 0.5) * m_dimensions.y);
		SDL_WarpMouseInWindow(m_window, x, y);
		TriggerMove(position, glm::ivec2(x, y));
	}

	virtual void SetPixelPosition(const glm::ivec2& position) override
	{
		SDL_WarpMouseInWindow(m_window, position.x, position.y);
		m_pixelPosition = position;
		TriggerMove(((glm::vec2(position) / m_dimensions) * 2.0f - 1.0f) * glm::vec2(1.0f, -1.0f), position);
	}
};