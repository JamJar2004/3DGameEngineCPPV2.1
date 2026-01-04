
#include <random>

#include <Engine/Platform/SDL/SDLApplication.hpp>

#include <Engine/Core/Game.hpp>
#include <Engine/EngineComponents/Transformation.hpp>
#include <Engine/Rendering/UserInterface/DefaultStyle.hpp>
#include <Engine/EngineSystems/DeferredRendererSystem.hpp>

namespace std
{
	std::mt19937 RandomGenerator(time(nullptr));

	static float randf() { return RandomGenerator() / float(std::numeric_limits<uint32_t>().max()); }

	static glm::vec2 rand2f() { return glm::vec2(randf(), randf()); }
	static glm::vec3 rand3f() { return glm::vec3(randf(), randf(), randf()); }
	static glm::vec4 rand4f() { return glm::vec4(randf(), randf(), randf(), randf()); }
}

class GUIScene : public Scene
{
public:
	MaterialHandle DefaultMaterial;

	MeshHandle CubeMesh;

	FontHandle ArialFont;

	GUIScene() : Scene(1000) {}

	// Inherited via Scene
	Camera OnStart(KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		AddSystem<DeferredRendererSystem>();

		ShaderHandle specularShader = LoadShader("deferred/specular.glsl", "deferred/specular_FS.glsl");
		specularShader->GetMaterialField("Color").SetDefaultValue(glm::vec3(1.0f));

		TextureHandle blankTexture = LoadTexture<glm::u8vec3>("blank.png");

		DefaultMaterial = CreateMaterial(specularShader);
		DefaultMaterial->SetTexture("Texture", blankTexture);

		CubeMesh = CreateCube();

		ArialFont = LoadFont("Arial");

		Transformation lightTransformation;
		lightTransformation.Rotate(glm::vec3(1, 0, 0), -45.0f);
		lightTransformation.Rotate(glm::vec3(0, 1, 0), 180.0f);
		CreateEntity(std::move(lightTransformation), CreateLight<DirectionalLight>(glm::vec3(1.0f), 1.0f));

		CreateEntity(Transformation(), CreateLight<AmbientLight>(glm::vec3(1.0f), 0.8f));

		return CreateCamera(Transformation(), Projection(70.0f, GraphicsMode().GetAspectRatio(), 0.1f, 1000.0f));
	}

	void OnUpdate(float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
	}

	void OnRender(RenderDevice& renderDevice, RenderContext2D& renderContext2D, FrameBufferHandle target) override
	{
	}

	float ButtonHeight = 100;

	virtual void OnUIRender(UIContext& context) override 
	{
		StackPanelContext stackPanel = context.RootContainer.BeginStackPanel(Orientation::Vertical);

		stackPanel.NextControl(120).Button("Button 1");
		stackPanel.NextControl(120).Button("Button 2");
		stackPanel.NextControl(120).Button("Button 3");
		
		if(stackPanel.NextControl(120).Button("Button 4").IsClicked)
		{
			glm::vec3 randomPosition = glm::vec3(std::rand2f() * 8.0f - 4.0f, -5.0f); 
			CreateEntity(Transformation(randomPosition, glm::quat(1, 0, 0, 0), glm::vec3(0.1f)), RenderableMesh(CubeMesh, DefaultMaterial));
		}

		StackPanelContext nestedPanel = stackPanel.NextControl(120).BeginStackPanel(Orientation::Horizontal);
		nestedPanel.NextControl(400).Button("Button 5");
		nestedPanel.NextControl(400).Button("Button 6");

		stackPanel.NextControl(120).Button("Button 5");

		//if(context.RootContainer.Button("Hello World").IsClicked)
		//{
		//	//glm::vec3 randomPosition = glm::vec3(std::rand2f() * 8.0f - 4.0f, -5.0f); 
		//	//CreateEntity(Transformation(randomPosition, glm::quat(1, 0, 0, 0), glm::vec3(0.1f)), RenderableMesh(CubeMesh, DefaultMaterial));
		//}
	}
};

GameSettings Game::StartSettings()
{
	SceneHandle startScene = std::make_shared<GUIScene>();
	return GameSettings(GraphicsMode(800, 600), "3D Game Engine GUI Test", startScene, 5000.0f);
}

