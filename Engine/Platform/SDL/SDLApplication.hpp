#pragma once

#include "../../Core/Game.hpp"
#include "../OpenGL/OpenGLRenderDevice.hpp"

#include "SDLInput.hpp"
#include "SDLAudio.hpp"

class SDLApplication : public Application
{
private:
	SDL_Window*   m_window;
	SDL_GLContext m_context;

	SDLKeyboardDevice* m_keyboard;
	SDLMouseDevice*    m_mouse;

	OpenGLRenderDevice* m_renderDevice;

	SDLAudioDevice* m_audioDevice;

	GraphicsMode m_graphicsMode;
public:
	SDLApplication(const std::string& title, GraphicsMode graphicsMode);
	~SDLApplication();

	virtual GraphicsMode GetGraphicsMode() const { return m_graphicsMode; }

	virtual KeyboardDevice& GetKeyboard()     override { return *m_keyboard;     }
	virtual MouseDevice&    GetMouse()        override { return *m_mouse;        }
	virtual RenderDevice&   GetRenderDevice() override { return *m_renderDevice; }
	virtual AudioDevice&    GetAudioDevice()  override { return *m_audioDevice;  }

	virtual bool ProcessEvents() override;

	virtual void SwapBuffers() override;
};

int main(int argc, char** argv);
