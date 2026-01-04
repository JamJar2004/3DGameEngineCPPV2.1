#include "Game.hpp"

std::vector<Timer*> Timer::s_timers;

void Game::Start(Application& app) const
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

	std::vector<ScreenVertex> vertices =
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

	std::stack<SceneHandle> scenes;
	scenes.push(Settings.StartScene);
	Settings.StartScene->Start(&app, &scenes);

	SceneHandle startScene = Settings.StartScene;

	ShaderHandle screenShader = startScene->RootAssetFolder.LoadShader("screen.glsl");

	AssetFolder shapesFolder = startScene->RootAssetFolder.Navigate("shapes");

	ShaderHandle rectangleShader = shapesFolder.LoadShader("rectangle.glsl");
	ShaderHandle   ellipseShader = shapesFolder.LoadShader("ellipse.glsl");
	ShaderHandle      textShader = shapesFolder.LoadShader("text.glsl");

	RenderContext2D renderContext(renderDevice, renderDevice.ScreenBuffer, rectangleShader, ellipseShader, textShader, quad);

	FontHandle defaultFont = Settings.StartScene->RootAssetFolder.LoadFont("Arial");

	Theme defaultTheme;
	defaultTheme.PushStyle<ButtonStyle, DefaultButtonStyle>(defaultFont, 30.0f);
	defaultTheme.PushStyle<DragStyle, DefaultDragStyle>(defaultFont, 30.0f);
	defaultTheme.PushStyle<ProgressBarStyle, DefaultProgressBarStyle>(defaultFont, 30.0f);

	UIContext uiContext(app.GetKeyboard(), app.GetMouse(), renderContext, defaultTheme);

	const Duration frameTime = Duration::FromSeconds(1) / Settings.FrameRate;

	uint32_t fps = 0;
	auto lastTime = Clock::CurrentTime();
	auto fpsTimeCounter = Duration::Zero();
	auto updateTimer = Duration::FromSeconds(1);

	bool running = true;
	while(running)
	{
		auto currentTime = Clock::CurrentTime();
		auto passedTime = currentTime - lastTime;
		lastTime = currentTime;

		updateTimer    += passedTime;
		fpsTimeCounter += passedTime;

		uiContext.ShouldRender = false;
		while(updateTimer >= frameTime)
		{
			uiContext.ShouldRender = false;

			updateTimer -= frameTime;

			running = running && app.ProcessEvents();

			scenes.top()->Update(static_cast<float>(frameTime.TotalSeconds()), app.GetKeyboard(), app.GetMouse());
			scenes.top()->UpdateAndRenderUI(uiContext);

			uiContext.ShouldRender = true;

			app.GetKeyboard().Update();
			app.GetMouse().Update();

			if(fpsTimeCounter.TotalSeconds() >= 1.0)
			{
				std::cout << "FPS: " << fps << std::endl;

				for(Timer* timer : Timer::GetTimers())
				{
					std::cout << timer->Name << ": " << timer->GetSamplesAverage().TotalMilliSeconds() << "ms" << std::endl;
				}
				fpsTimeCounter = Duration::Zero();
				fps = 0;
			}
		}

		if(uiContext.ShouldRender)
		{
			{
				ScopeTimer timer(totalRenderTimer);
				//renderDevice.Enable(DepthTest);
				scenes.top()->Render(renderDevice, renderContext, *renderDevice.ScreenBuffer);//primaryFrameBuffer);

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
