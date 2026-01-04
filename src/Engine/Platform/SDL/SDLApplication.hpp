#pragma once

#include "../../Core/Game.hpp"
#include "../OpenGL/OpenGLRenderDevice.hpp"

#include "SDLInput.hpp"
#include "SDLAudio.hpp"

class SDLApplication final : public Application
{
public:
	SDLApplication(const std::string& title, ScreenGraphicsMode graphicsMode);

	~SDLApplication() override;

	ScreenGraphicsMode GetGraphicsMode() const override { return m_graphicsMode; }

	KeyboardDevice& GetKeyboard()     override { return *m_keyboard;     }
	MouseDevice&    GetMouse()        override { return *m_mouse;        }
	RenderDevice&   GetRenderDevice() override { return *m_renderDevice; }
	AudioDevice&    GetAudioDevice()  override { return *m_audioDevice;  }

	bool ProcessEvents() override;

	void SwapBuffers() override;
private:
	SDL_Window*   m_window;
	SDL_GLContext m_context;

	SDLKeyboardDevice* m_keyboard;
	SDLMouseDevice*    m_mouse;

	OpenGLRenderDevice* m_renderDevice;

	SDLAudioDevice* m_audioDevice;

	ScreenGraphicsMode m_graphicsMode;
};

int main(int argc, char** argv);
