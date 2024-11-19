#include "Game.hpp"

std::vector<Timer*> Timer::s_timers;

void Game::Start(const GameSettings& settings, Application& app)
{
	Timer totalRenderTimer("Total Render Time");

	RenderDevice& renderDevice = app.GetRenderDevice();

	/*FrameBufferHandle primaryFrameBuffer = renderDevice.CreateFrameBuffer(settings.GraphicsMode.Width, settings.GraphicsMode.Height,
	{
	AttachmentInfo(InternalFormat::RGB32F, Format::RGB, ElementType::Float32, MinFilterMode::Linear, MagFilterMode::Linear, WrappingMode::ClampedToEdge),
	}, {});*/

	//FrameBufferHandle primaryFrameBuffer = renderDevice.CreateFrameBuffer(settings.GraphicsMode.Width, settings.GraphicsMode.Height,
	//{
	//	AttachmentInfo(InternalFormat::RGB8, Format::RGB, ElementType::UInt8, MinFilterMode::Nearest, MagFilterMode::Nearest, WrappingMode::ClampedToEdge),
	//}, {});

	Array<ScreenVertex> vertices =
	{
		glm::vec2(-1, -1),
		glm::vec2( 1, -1),
		glm::vec2(-1,  1),
		glm::vec2( 1,  1),
	};

	std::vector<uint32_t> indices =
	{
		0, 2, 1,
		1, 2, 3
	};

	Model quadModel(vertices, indices);

	MeshHandle quad = app.GetRenderDevice().CreateMesh(quadModel);

	VertexArrayHandle quadVertexArray = quad->CreateVertexArray(BufferLayout());

	std::stack<SceneHandle> scenes;
	scenes.push(settings.StartScene);
	settings.StartScene->Start(&app, &scenes);

	ShaderHandle screenShader = settings.StartScene->LoadShader("screen_VS.glsl", "screen_FS.glsl");

	ShaderHandle rectangleShader = settings.StartScene->LoadShader("shapes/rectangle_VS.glsl", "shapes/rectangle_FS.glsl");
	ShaderHandle   ellipseShader = settings.StartScene->LoadShader("shapes/ellipse_VS.glsl", "shapes/ellipse_FS.glsl");
	ShaderHandle      textShader = settings.StartScene->LoadShader("shapes/text_VS.glsl", "shapes/text_FS.glsl");

	RenderContext2D renderContext(renderDevice, rectangleShader, ellipseShader, textShader, quad);

	FontHandle defaultFont = settings.StartScene->LoadFont("Arial");

	Theme defaultTheme;
	defaultTheme.PushStyle<ButtonStyle, DefaultButtonStyle>(defaultFont, 30.0f);
	defaultTheme.PushStyle<DragStyle, DefaultDragStyle>(defaultFont, 30.0f);
	defaultTheme.PushStyle<ProgressBarStyle, DefaultProgressBarStyle>(defaultFont, 30.0f);

	UIContext uiContext(app.GetKeyboard(), app.GetMouse(), renderContext, defaultTheme);

	const double frameTime = 1.0f / settings.FrameRate;

	double timer = 0;

	uint32_t fps = 0;
	auto lastTime = std::chrono::system_clock::now();
	double fpsTimeCounter = 0;
	double updateTimer = 1;

	bool running = true;
	while(running)
	{
		auto currentTime = std::chrono::system_clock::now();
		double passedTime = std::chrono::duration<double>(currentTime - lastTime).count();
		lastTime = currentTime;

		updateTimer += passedTime;
		fpsTimeCounter += passedTime;

		uiContext.ShouldRender = false;
		while(updateTimer >= frameTime)
		{
			uiContext.ShouldRender = false;

			updateTimer -= frameTime;

			running = running && app.ProcessEvents();

			scenes.top()->Update(float(frameTime), app.GetKeyboard(), app.GetMouse());
			scenes.top()->UpdateAndRenderUI(uiContext);

			uiContext.ShouldRender = true;

			app.GetKeyboard().Update();
			app.GetMouse().Update();

			if(fpsTimeCounter >= 1.0f)
			{
				std::cout << "FPS: " << fps << std::endl;

				for(Timer* timer : Timer::GetTimers())
				{
					std::cout << timer->Name << ": " << timer->GetSamplesAverage() << std::endl;
				}
				fpsTimeCounter = 0.0f;
				fps = 0;
			}
		}

		if(uiContext.ShouldRender)
		{
			{
				ScopeTimer timer(totalRenderTimer);
				//renderDevice.Enable(DepthTest);
				scenes.top()->Render(renderDevice, renderContext, renderDevice.ScreenBuffer);//primaryFrameBuffer);

				renderContext.Flush();
			}
			//renderDevice.ScreenBuffer->Bind();
			//renderDevice.ClearScreen(ColorBuffer | DepthBuffer);

			//renderDevice.Disable(DepthTest);

			//screenShader->SetTexture("u_source", primaryFrameBuffer->GetColorAttachment(0));
			//screenShader->Use(nullptr);
			//quadVertexArray->Draw(nullptr);

			scenes.top()->UpdateAndRenderUI(uiContext);

			app.SwapBuffers();
			fps++;
		}
	}

}
