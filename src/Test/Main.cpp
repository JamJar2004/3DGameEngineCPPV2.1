#include <Engine/Platform/SDL/SDLApplication.hpp>

#include <Engine/EngineComponents/Transformation.hpp>
#include <Engine/EngineComponents/RenderableMesh.hpp>
#include <Engine/EngineComponents/LightComponent.hpp>
#include <Engine/EngineComponents/AnimationComponent.hpp>

#include <Engine/EngineSystems/MouseLookSystem.hpp>
#include <Engine/EngineSystems/KeyboardMovementSystem.hpp>
#include <Engine/EngineSystems/RotaterSystem.hpp>
#include <Engine/EngineSystems/MovementSystem.hpp>
#include <Engine/EngineSystems/FollowerSystem.hpp>
#include <Engine/EngineSystems/WaterUpdaterSystem.hpp>
#include <Engine/EngineSystems/AnimationSystem.hpp>
#include <Engine/EngineSystems/AudioUpdaterSystem.hpp>
#include <Engine/EngineSystems/DeferredRendererSystem.hpp>
#include <Engine/EngineSystems/SkyboxRendererSystem.hpp>
#include <Engine/EngineSystems/WaterRendererSystem.hpp>
#include <Engine/EngineSystems/BloomRendererSystem.hpp>

namespace std
{
	static float randf() { return rand() / float(RAND_MAX); }

	static glm::vec2 rand2f() { return glm::vec2(randf(), randf());                   }
	static glm::vec3 rand3f() { return glm::vec3(randf(), randf(), randf());          }
	static glm::vec4 rand4f() { return glm::vec4(randf(), randf(), randf(), randf()); }
}

static glm::vec3 CreateHSL(float hue, float saturation, float luminance)
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

	float h = fmodf(hue, 360.0f);

	uint32_t section = uint32_t(h / 60);
	float varying = (h / 60.0f) - section;

	switch(section)
	{
		case 0:
			r = 1.0f;
			g = varying;
			b = 0.0f;
			break;
		case 1:
			r = 1.0f - varying;
			g = 1.0f;
			b = 0.0f;
			break;
		case 2:
			r = 0.0f;
			g = 1.0f;
			b = varying;
			break;
		case 3:
			r = 0.0f;
			g = 1.0f - varying;
			b = 1.0f;
			break;
		case 4:
			r = varying;
			g = 0;
			b = 1.0f;
			break;
		case 5:
			r = 1.0f;
			g = 0;
			b = 1.0f - varying;
			break;
	}

	if(luminance <= 0.5f)
    {
        r *= (luminance * 2.0f);
        g *= (luminance * 2.0f);
        b *= (luminance * 2.0f);
    }
    else
    {
        float value = (luminance - 0.5f) * 2.0f;
        r += value - (value * r);
        g += value - (value * g);
        b += value - (value * b);
    }

	r = (r * saturation) + (luminance * (1.0f - saturation));
    g = (g * saturation) + (luminance * (1.0f - saturation));
    b = (b * saturation) + (luminance * (1.0f - saturation));
	
	return glm::vec3(r, g, b);
}

class TestScene : public Scene
{
public:
	TestScene() : Scene(10000), TextBuffer{}, CursorIndex(0) {}

	char TextBuffer[1024];

	std::string_view Text;

	size_t CursorIndex;

	FontHandle ArialFont;
	FontHandle ConsolasFont;

	virtual Camera OnStart(KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		Text = std::string_view(TextBuffer, CursorIndex);

		Camera result = CreateCamera(Transformation(glm::vec3(0, 2, 3)), Projection(70.0f, SelectedGraphicsMode().GetAspectRatio(), 0.1f, 1000.0f));
		result.AddComponent<MouseLookComponent>(0.5f);
		KeyboardMovementComponent& controller = result.AddComponent<KeyboardMovementComponent>();
		controller.SetKeyControl(Key::W, glm::vec3( 0, 0, -1), 10.0f);
		controller.SetKeyControl(Key::S, glm::vec3( 0, 0,  1), 10.0f);
		controller.SetKeyControl(Key::A, glm::vec3(-1, 0,  0), 10.0f);
		controller.SetKeyControl(Key::D, glm::vec3( 1, 0,  0), 10.0f);

		AddSystem<KeyboardMovementSystem>();
		AddSystem<MouseLookSystem>().IsEnabled = false;
		AddSystem<RotaterSystem>();
		AddSystem<MovementSystem>();
		AddSystem<FollowerSystem>();
		AddSystem<WaterUpdaterSystem>();
		AddSystem<AnimationSystem<Transformation>>();
		AddSystem<AnimationSystem<glm::vec3>>();
		AddSystem<AudioUpdaterSystem>();

		DeferredRendererSystem& rendererSystem = AddSystem<DeferredRendererSystem>(glm::uvec2(1024));
		AddSystem<SkyboxRendererSystem>();
		AddSystem<WaterRendererSystem>(glm::uvec2(1024));
		//AddSystem<BloomRendererSystem>();

		ShaderHandle skyboxShader = LoadShader("skybox_VS.glsl", "skybox_FS.glsl");
		skyboxShader->GetMaterialField("Emission").SetDefaultValue(1.0f);

		MaterialHandle cubeMap = CreateMaterial(skyboxShader);
		cubeMap->SetTexture("CubeMap", LoadCubeMap<glm::u8vec3>("left.png", "right.png", "bottom.png", "top.png", "back.png", "front.png"));

		CreateEntity(Transformation(glm::vec3(0), glm::quat(1, 0, 0, 0), glm::vec3(300)), SkyboxComponent(cubeMap), FollowerComponent(result, glm::vec3()));

		ShaderHandle specularShader = LoadShader("deferred/specular.glsl", "deferred/specular_FS.glsl");
		specularShader->GetMaterialField("Color").SetDefaultValue(glm::vec3(1.0f));
		specularShader->GetMaterialField("SpecularIntensity").SetDefaultValue(0.0f);
		specularShader->GetMaterialField("TilingFactor").SetDefaultValue(glm::vec2(1.0f));

		ShaderHandle normalMappedShader = LoadShader("deferred/normalMapped.glsl", "deferred/normalMapped_FS.glsl");
		normalMappedShader->GetMaterialField("Color").SetDefaultValue(glm::vec3(1.0f));
		normalMappedShader->GetMaterialField("SpecularIntensity").SetDefaultValue(0.0f);
		normalMappedShader->GetMaterialField("TilingFactor").SetDefaultValue(glm::vec2(1.0f));

		MeshHandle terrain = LoadTerrain("heightmap.png");

		MeshHandle monkeyMesh = LoadMesh("monkey3.obj");

		MaterialHandle material = CreateMaterial(normalMappedShader);
		material->SetTexture("Texture", LoadTexture<glm::u8vec3>("bricks2.jpg"));
		material->SetTexture("NormalMap", LoadTexture<glm::u8vec3>("bricks2_normal.jpg"));
		material->Get<float>("SpecularIntensity") = 0.0f;
		material->Get<glm::vec2>("TilingFactor") = glm::vec2(10.0f);

		TextureHandle blankTexture = LoadTexture<glm::u8vec3>("blank.png");
		MaterialHandle monkeyMaterial = CreateMaterial(specularShader);
		monkeyMaterial->Get<glm::vec3>("Color") = glm::vec3(0.5f, 0.2f, 1.0f);
		monkeyMaterial->SetTexture("Texture", blankTexture);
		monkeyMaterial->Get<float>("SpecularIntensity") = 1.0f;

		const float terrainSize = 20.0f;
		ECS::Entity terrainEntity = CreateEntity(Transformation(glm::vec3(), glm::quat(1, 0, 0, 0), glm::vec3(terrainSize)), RenderableMesh(terrain, material));

		MeshHandle sphere = CreateSphere();

		//MusicHandle fateOfTheStars = LoadMusic("Fate of the stars.wav");
		MusicHandle whiteBall = LoadMusic("White Ball.mp3");

		//PlayMusic(whiteBall, false);

		AudioSourceHandle audioSource = CreateAudioSource();

		SoundHandle testSound = LoadSound("Kieran's scream.wav");

		//audioSource->Play(testSound, true);

		ECS::Entity sphereEntity = CreateEntity(Transformation(glm::vec3(-1.0f * terrainSize, 5.0f, sinf(0) * 5)), RenderableMesh(monkeyMesh, monkeyMaterial), AudioSourceComponent(audioSource, Attenuation(0, 0, 0.25f)));
		auto& animation = sphereEntity.AddComponent<AnimationComponent<Transformation>>(sphereEntity.GetComponent<Transformation>(), 1.0f, true);
		animation.Loop = true;

		auto& colorAnimation = sphereEntity.AddComponent<AnimationComponent<glm::vec3>>(monkeyMaterial->Get<glm::vec3>("Color"), 1.0f, true);
		colorAnimation.Loop = true;

		constexpr size_t MONKEY_COUNT = 10;
		for(size_t i = 1; i < MONKEY_COUNT; i++)
		{
			float hue = (i / float(MONKEY_COUNT)) * 360.0f;

			float x = (i / float(MONKEY_COUNT)) * 2.0f - 1.0f;
			glm::quat rotation = glm::angleAxis(glm::radians(i * (360.0f / MONKEY_COUNT)), glm::vec3(0, 1, 0));
			CreateEntity(Transformation(glm::vec3(x * terrainSize, 5, sinf(i) * 5), rotation), RenderableMesh(monkeyMesh, monkeyMaterial), RotaterComponent(glm::vec3(0, 1, 0), 40));
		
			animation.AddFrame(1.0f, Transformation(glm::vec3(x * terrainSize, 5, sinf(i) * 5), rotation));

			colorAnimation.AddFrame(1.0f, CreateHSL(hue, 1.0f, 0.5f));
		}
		
		CreateEntity(Transformation(), CreateLight<AmbientLight>(glm::vec3(1), 0.1f));

		Transformation lightTransformation;
		lightTransformation.Rotate(glm::vec3(1, 0, 0), -45.0f);
		lightTransformation.Rotate(glm::vec3(0, 1, 0), 135.0f);
		CreateEntity(lightTransformation, CreateLight<DirectionalLight>(glm::vec3(1.0f), 1.0f, 5.0f));

		//CreateEntity(Transformation(glm::vec3(0.0f, -0.5f, 0.0f)), CreateLight<PointLight>(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, Attenuation(0, 0, 1)));

		CreateEntity(Transformation(glm::vec3(2.0f, -0.5f, 0.0f), glm::angleAxis(-20.0f, glm::vec3(1, 0, 0))), CreateLight<PointLight>(glm::vec3(0.0f, 1.0f, 1.0f), 1.0f, Attenuation(0, 0, 0.1f)));
		//CreateEntity(Transformation(glm::vec3(2.0f, -0.5f, 0.0f), glm::angleAxis(-20.0f, glm::vec3(1, 0, 0))), CreateLight<SpotLight>(glm::vec3(0.0f, 1.0f, 1.0f), 1.0f, Attenuation(0, 0, 0.1f), 90.0f));

		//Transformation lightTransformation1;
		//lightTransformation1.Rotate(glm::vec3(1, 0, 0),  -45.0f);
		//lightTransformation1.Rotate(glm::vec3(0, 1, 0), 0.0f);
		//CreateEntity(std::move(lightTransformation1), CreateLight<DirectionalLight>(glm::vec3(1, 0, 0), 1.0f));
		//
		//Transformation lightTransformation2;
		//lightTransformation2.Rotate(glm::vec3(1, 0, 0), -45.0f);
		//lightTransformation2.Rotate(glm::vec3(0, 1, 0), 90.0f);
		//CreateEntity(std::move(lightTransformation2), CreateLight<DirectionalLight>(glm::vec3(0, 1, 0), 1.0f));
		//
		//Transformation lightTransformation3;
		//lightTransformation3.Rotate(glm::vec3(1, 0, 0), -45.0f);
		//lightTransformation3.Rotate(glm::vec3(0, 1, 0), 180.0f);
		//CreateEntity(std::move(lightTransformation3), CreateLight<DirectionalLight>(glm::vec3(0, 0, 1), 1.0f));

		ShaderHandle diffuseShader = LoadShader("deferred/diffuse_VS.glsl", "deferred/diffuse_FS.glsl");
		diffuseShader->GetMaterialField("Color").SetDefaultValue(glm::vec3(1));

		/*MaterialHandle diffuseMaterial = CreateMaterial(diffuseShader);
		diffuseMaterial->Get<glm::vec3>("Color") = glm::vec3(1.0f, 1.0f, 1.0f) * 10.0f;
		diffuseMaterial->SetTexture("Texture", blankTexture);

		CreateEntity(Transformation(glm::vec3(170.0f, 300.0f, -130.0f), glm::quat(1, 0, 0, 0), glm::vec3(20.0f)), RenderableMesh(sphere, diffuseMaterial, true));*/

		ShaderHandle waterShader = LoadShader("deferred/water/water_VS.glsl", "deferred/water/water_FS.glsl");
		waterShader->GetMaterialField("WaveStrength").SetDefaultValue(0.04f);
		waterShader->GetMaterialField("SpecularIntensity").SetDefaultValue(1.3f);

		MaterialHandle waterMaterial = CreateMaterial(waterShader);
		waterMaterial->SetTexture("DistortionMap", LoadTexture<glm::u8vec3>("dudvMap.png"));
		waterMaterial->SetTexture("NormalMap", LoadTexture<glm::u8vec3>("normalMap.png"));
		CreateEntity(Transformation(glm::vec3(0, -1.0f, 0), glm::quat(1, 0, 0, 0), glm::vec3(terrainSize)), WaterComponent(waterMaterial));

		//AudioSourceHandle audioSource = CreateAudioSource();

		//ShaderHandle rectangleShader = LoadShader("shapes/shape_VS.glsl", "shapes/rectangle_FS.glsl");
		//ShaderHandle ellipseShader = LoadShader("shapes/shape_VS.glsl", "shapes/ellipse_FS.glsl");

		//Rectangle = CreateMaterial(rectangleShader);
		//Ellipse = CreateMaterial(ellipseShader);
		// 
		//SolidColorBrush = CreateMaterial(solidColorBrushShader);
		//SolidColorBrush.Material->Get<glm::vec4>("Color") = glm::vec4(1, 1, 0, 0.5f);

		mouse.OnButtonDown += [this](MouseButton button)
		{
			if(button == MouseButton::Left)
			{
				EnableSystem<MouseLookSystem>();
			}
		};

		mouse.OnButtonUp += [this](MouseButton button)
		{
			if(button == MouseButton::Left)
			{
				DisableSystem<MouseLookSystem>();
			}
		};

		keyboard.OnKeyDown += [this](Key key)
		{
			if(key >= Key::A && key <= Key::Z)
			{
				TextBuffer[CursorIndex++] = (char)key;
			}
			else if(key >= Key::Num_0 && key <= Key::Num_9)
			{
				TextBuffer[CursorIndex++] = '0' + (char)key;
			}
			else if(key == Key::Space)
			{
				TextBuffer[CursorIndex++] = ' ';
			}
			else if(key == Key::Backspace && CursorIndex > 0)
			{
				--CursorIndex;
			}

			Text = std::string_view(TextBuffer, CursorIndex);
		};

		ArialFont = LoadFont("Arial");
		ConsolasFont = LoadFont("Consolas");
		return result;
	}
	
	virtual void OnUpdate(float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		if(keyboard.GetKeyState(Key::M) == ButtonState::Pressed)
		{
			TogglePauseMusic();
			SetMusicPosition(0.0f);
		}
	}

	virtual void OnRender(RenderDevice& renderDevice, RenderContext2D& renderContext, FrameBufferHandle target) override
	{
		renderContext.DrawEllipse(glm::vec2(0, 0), glm::vec2(100, 200), 4, glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 0.5f));
		renderContext.DrawRectangle(glm::vec2(50, 100), glm::vec2(100, 200), 10, 0.0f, glm::vec4(0, 0, 1, 1), glm::vec4(1, 0, 1, 0.5f));
	
		//renderContext.DrawText(glm::vec2(0, 0), 1.0f, "[Hello World!]", ArialFont);
		renderContext.DrawText(glm::vec2(0, 0), Text, ArialFont, 24.0f, 0.7f, 0.4f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		//renderContext.DrawText(glm::vec2(100, 100), 1.0f, "[Hello World!]", ArialFont);
		//renderContext.DrawText(glm::vec2(100, 200), 1.0f, "[Hello World!]", ConsolasFont);
	}
};

GameSettings Game::StartSettings()
{
	SceneHandle startScene = std::make_shared<TestScene>();
	return GameSettings(ScreenGraphicsMode(800, 600), "3D Game Engine Test", startScene, 5000.0f);
}
