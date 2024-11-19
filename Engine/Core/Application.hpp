#pragma once

#include "Input.hpp"
#include "../Rendering/RenderDevice.hpp"
#include "../Audio/AudioDevice.hpp"

class Application
{
public:
	virtual ~Application() {}

	virtual GraphicsMode GetGraphicsMode() const = 0;

	virtual KeyboardDevice& GetKeyboard()     = 0;
	virtual    MouseDevice& GetMouse()        = 0;
	virtual   RenderDevice& GetRenderDevice() = 0;
	virtual    AudioDevice& GetAudioDevice()  = 0;

	virtual bool ProcessEvents() = 0;
	virtual void SwapBuffers() = 0;
};