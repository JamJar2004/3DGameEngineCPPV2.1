#include "SDLApplication.hpp"

int main(int argc, char** argv)
{
	Game game;
	GameSettings settings = game.StartSettings();
	SDLApplication app(settings.Title, settings.GraphicsMode);
	game.Start(settings, app);
	return 0;
}

SDLApplication::SDLApplication(const std::string& title, GraphicsMode graphicsMode) : 
	m_window(nullptr), m_context(nullptr), m_keyboard(nullptr), m_mouse(nullptr), m_renderDevice(nullptr), m_graphicsMode(graphicsMode)
{
	if(SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		m_window  = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, graphicsMode.Width, graphicsMode.Height, SDL_WINDOW_OPENGL);
		m_context = SDL_GL_CreateContext(m_window);
			
		m_keyboard = new SDLKeyboardDevice();
		m_mouse    = new SDLMouseDevice(graphicsMode, m_window);

		m_renderDevice = new OpenGLRenderDevice(graphicsMode);

		m_audioDevice = new SDLAudioDevice();
	}
}

SDLApplication::~SDLApplication()
{
	delete m_keyboard;
	delete m_mouse;

	delete m_renderDevice;

	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);

	SDL_Quit();
}

bool SDLApplication::ProcessEvents()
{
	bool result = true;

	SDL_Event e;
	while(SDL_PollEvent(&e))
	{
		switch(e.type)
		{ 
			case SDL_QUIT:
				result = false;
				break;
		}
		m_keyboard->OnEvent(e);
		m_mouse->OnEvent(e);
	}

	return result;
}

void SDLApplication::SwapBuffers()
{
	SDL_GL_SwapWindow(m_window);
}
