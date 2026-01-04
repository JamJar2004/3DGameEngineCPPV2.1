#pragma once

#include <stack>

#include "Application.hpp"
#include "Input.hpp"
#include "Camera.hpp"
#include "System.hpp"
#include "../Rendering/RenderDevice.hpp"
#include "../Rendering/RenderContext2D.hpp"
#include "../Rendering/UserInterface/UIContext.hpp"

#include <ECS/Scene.hpp>

#include "AssetFolder.hpp"

class Application;
class Scene;

using SceneHandle = std::shared_ptr<Scene>;

struct DefaultVertex
{
	DefaultVertex(const glm::vec3& position, const glm::vec2& texCoord, const glm::vec3& normal, const glm::vec3& tangent = glm::vec3()) :
		Position(position), TexCoord(texCoord), Normal(normal), Tangent(tangent) {}

	glm::vec3 Position;
	glm::vec2 TexCoord;
	glm::vec3 Normal;
	glm::vec3 Tangent;

	static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(Position)>();
		result.AddAttribute<decltype(TexCoord)>();
		result.AddAttribute<decltype(Normal)>();
		result.AddAttribute<decltype(Tangent)>();
		return result;
	}
};

struct ScreenGraphicsMode;

class HeightMap
{
public:
	explicit HeightMap(const BitmapHandle<glm::u8vec1>& bitmap) :
		SourceBitmap(*bitmap, [](const glm::u8vec1 value) { return glm::vec1(value) / 255.0f; }) {}

	const Bitmap<glm::vec1> SourceBitmap;

	[[nodiscard]] float Get(uint32_t x, uint32_t y) const { return SourceBitmap.GetPixel(x, y).x; }
};

class Scene : public ECS::Scene
{

protected:
	virtual Camera OnStart(KeyboardDevice& keyboard, MouseDevice& mouse) = 0;
	virtual void   OnUpdate(float delta, KeyboardDevice& keyboard, MouseDevice& mouse) = 0;
	virtual void   OnRender(RenderDevice& renderDevice, RenderContext2D& renderContext2D, RenderTarget& target) = 0;
	
	virtual void OnUIRender(UIContext& context) {}
public:
	explicit Scene(AssetLoaders& assetLoaders, size_t maxEntities) :
		ECS::Scene(maxEntities),
		m_app(nullptr), 
		m_scenes(nullptr),
		m_isMusicPaused(false),
		RootAssetFolder(assetLoaders, std::filesystem::path("assets")) {}

	void Render(RenderDevice& renderDevice, RenderContext2D& renderContext2D, RenderTarget& target);

	void UpdateAndRenderUI(UIContext& context);

	ScreenGraphicsMode SelectedGraphicsMode() const;

	MeshHandle GetScreenQuad() const { return m_screenQuad; }

	template<typename T, typename... Args>
	SceneHandle OpenScene(Args&&... args) requires std::constructible_from<T, Args...>;

	void Close();

	template<std::derived_from<ECS::ComponentBase>... Rest>
	Camera CreateCamera(Transformation&& transformation, Projection&& projection, Rest&&... rest)
	{
		const ECS::Entity entity = CreateEntity(std::move(transformation), std::move(projection), std::forward<Rest>(rest)...);
		return Camera(entity);
	}

	Camera PrimaryCamera;

	template<std::derived_from<UpdaterSystem> TSystem, typename... TArgs>
	TSystem& AddSystem(TArgs&&... args) requires std::constructible_from<TSystem, TArgs&&...>;

	template<std::derived_from<RendererSystem> TSystem, typename... TArgs>
	TSystem& AddSystem(TArgs&&... args) requires std::constructible_from<TSystem, TArgs&&...>;

	template<std::derived_from<UpdaterSystem> T>
	void EnableSystem();

	template<std::derived_from<UpdaterSystem> T>
	void DisableSystem();

	template<std::derived_from<RendererSystem> T>
	void EnableSystem();

	template<std::derived_from<RendererSystem> T>
	void DisableSystem();

	AudioSourceHandle CreateAudioSource();

	        MeshHandle CreateMesh(const Model& model) const;
	TextureAtlasHandle CreateTextureAtlas(const BitmapBase& image, MinFilterMode minFilter = MinFilterMode::LinearMipmapLinear, MagFilterMode magFilter = MagFilterMode::Linear, TextureWrappingMode wrappingMode = TextureWrappingMode::Tiled) const;

	TextureAtlasHandle CreateCubeMap(const BitmapBase& image,
	                                 MinFilterMode minFilter = MinFilterMode::LinearMipmapLinear,
	                                 MagFilterMode magFilter = MagFilterMode::Linear,
	                                 TextureWrappingMode wrappingMode = TextureWrappingMode::ClampedToEdge) const;
	RenderTargetHandle CreateRenderTarget(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachments) const;
		  ShaderHandle CreateShader(std::string_view sourceCode) const;
			FontHandle CreateFont(const BitmapBase& image, std::istream& jsonStream) const;
	       SoundHandle CreateSound(ConstBufferSlice<float> samples) const;

	MeshHandle CreateQuad() const;
	MeshHandle CreateCube() const;
	MeshHandle CreatePyramid() const;
	MeshHandle CreateCone(uint32_t detail = 60) const;
	MeshHandle CreateCylinder(uint32_t detail = 60) const;
	MeshHandle CreateSphere(uint32_t detail = 60) const;
	MeshHandle CreateTerrain(const HeightMap& heightMap) const;

	AssetFolder RootAssetFolder;

	void  PlayMusic(MusicHandle music, bool loop);
	void  StopMusic();
	void PauseMusic(bool pause);

	void TogglePauseMusic();

	bool IsMusicPaused() const { return m_isMusicPaused; }

	void SetMusicPosition(float seconds) { m_app->GetAudioDevice().SetMusicPosition(seconds); }

	friend struct Game;
private:
	Application* m_app;
	std::stack<SceneHandle>* m_scenes;

	MeshHandle m_screenQuad;

	std::vector<std::shared_ptr<UpdaterSystem>>  m_updaterSystems;
	std::vector<std::shared_ptr<RendererSystem>> m_rendererSystems;

	std::unordered_map<TypeInfo*, size_t> m_updaterSystemIndices;
	std::unordered_map<TypeInfo*, size_t> m_rendererSystemIndices;

	std::unordered_map<std::string, FontHandle> m_fonts;

	bool m_isMusicPaused;

	void Start(Application* app, std::stack<SceneHandle>* scenes);
	void Update(float delta, KeyboardDevice& keyboard, MouseDevice& mouse);
};

// template<typename Pixel>
// TextureHandle Scene::LoadCubeMap(const std::string& left, const std::string& right, const std::string& bottom, const std::string& top, const std::string& back, const std::string& front, const std::string& resourceFolderName, MinFilterMode minFilter, MagFilterMode magFilter, TextureWrappingMode wrappingMode)
// {
// 	return LoadCubeMap(left, right, bottom, top, back, front, PixelFormat::GetFormat<Pixel>(), resourceFolderName, minFilter, magFilter, wrappingMode);
// }

template<typename T, typename ...Args>
SceneHandle Scene::OpenScene(Args&&... args) requires std::constructible_from<T, Args...>
{
	SceneHandle result = std::make_shared<T>(args...);
	m_scenes->push(result);
	result->Start(m_app, m_scenes);
	return result;
}

template<std::derived_from<UpdaterSystem> TSystem, typename ...TArgs>
TSystem& Scene::AddSystem(TArgs&&... args) requires std::constructible_from<TSystem, TArgs&&...>
{
	std::shared_ptr<TSystem> system = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
	system->OnStart(*this);
	size_t index = m_updaterSystems.size();
	m_updaterSystems.push_back(system);
	m_updaterSystemIndices[TypeInfo::Get<TSystem>()] = index;
	return *system;
}

template<std::derived_from<RendererSystem> TSystem, typename... TArgs>
TSystem& Scene::AddSystem(TArgs&&... args) requires std::constructible_from<TSystem, TArgs&&...>
{
	std::shared_ptr<TSystem> system = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
	size_t index = m_rendererSystems.size();
	m_rendererSystems.push_back(system);
	m_rendererSystemIndices[TypeInfo::Get<TSystem>()] = index;
	return *system;
}

template<std::derived_from<UpdaterSystem> T>
inline void Scene::EnableSystem()
{
	size_t index = m_updaterSystemIndices[TypeInfo::Get<T>()];
	std::shared_ptr<UpdaterSystem> system = m_updaterSystems[index];
	system->IsEnabled = true;
}

template<std::derived_from<UpdaterSystem> T>
inline void Scene::DisableSystem()
{
	size_t index = m_updaterSystemIndices[TypeInfo::Get<T>()];
	std::shared_ptr<UpdaterSystem> system = m_updaterSystems[index];
	system->IsEnabled = false;
}

template<std::derived_from<RendererSystem> T>
inline void Scene::EnableSystem()
{
	size_t index = m_rendererSystemIndices[TypeInfo::Get<T>()];
	std::shared_ptr<RendererSystem> system = m_rendererSystems[index];
	system->IsEnabled = true;
}

template<std::derived_from<RendererSystem> T>
inline void Scene::DisableSystem()
{
	size_t index = m_rendererSystemIndices[TypeInfo::Get<T>()];
	std::shared_ptr<RendererSystem> system = m_rendererSystems[index];
	system->IsEnabled = false;
}
