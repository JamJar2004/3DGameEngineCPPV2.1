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

struct GraphicsMode;

class HeightMap
{
public:
	HeightMap(BitmapHandle<uint8_t> bitmap) : 
		Bitmap(*bitmap, [](glm::uint8_t value) { return value / 255.0f; }) {}

	const Bitmap<float> Bitmap;

	float Get(uint32_t x, uint32_t y) const { return Bitmap.GetPixel(x, y); }
};

class Shapes
{
private:

};

class Scene : public ECS::Scene
{
private:
	Application* m_app;
	std::stack<SceneHandle>* m_scenes;

	MeshHandle m_screenQuad;

	std::vector<std::shared_ptr<UpdaterSystem>>  m_updaterSystems;
	std::vector<std::shared_ptr<RendererSystem>> m_rendererSystems;

	std::unordered_map<Type*, size_t> m_updaterSystemIndices;
	std::unordered_map<Type*, size_t> m_rendererSystemIndices;

	std::unordered_map<ShaderHandle, std::shared_ptr<MaterialList>> m_materialLists;

	std::unordered_map<std::string, FontHandle> m_fonts;

	std::string m_resourcesPath;
	
	bool m_isMusicPaused;

	void Start(Application* app, std::stack<SceneHandle>* scenes);
	void Update(float delta, KeyboardDevice& keyboard, MouseDevice& mouse);
protected:
	virtual Camera OnStart(KeyboardDevice& keyboard, MouseDevice& mouse) = 0;
	virtual void   OnUpdate(float delta, KeyboardDevice& keyboard, MouseDevice& mouse) = 0;
	virtual void   OnRender(RenderDevice& renderDevice, RenderContext2D& renderContext2D, FrameBufferHandle target) = 0;
	
	virtual void OnUIRender(UIContext& context) {}
public:
	Scene(size_t maxEntities) : 
		ECS::Scene(maxEntities),
		m_app(nullptr), 
		m_scenes(nullptr), 
		m_resourcesPath("./res"),
		m_isMusicPaused(false) 
	{
	}

	void Render(RenderDevice& renderDevice, RenderContext2D& renderContext2D, FrameBufferHandle target);

	void UpdateAndRenderUI(UIContext& context);

	GraphicsMode GraphicsMode() const;

	MeshHandle GetScreenQuad() const { return m_screenQuad; }

	template<typename T, typename... Args>
	SceneHandle OpenScene(Args&&... args) requires std::constructible_from<T, Args...>;

	void Close();

	void AddToResourcesPath(const std::string& path)
	{
		m_resourcesPath += "/" + path;
	}

	void GoUpDirectory()
	{
		size_t lastSlashIndex = m_resourcesPath.find_last_of('/');
		m_resourcesPath = m_resourcesPath.substr(0, lastSlashIndex);
	}

	template<std::derived_from<ECS::BaseComponent>... Rest>
	Camera CreateCamera(Transformation&& transformation, Projection&& projection, Rest&&... rest)
	{
		ECS::Entity entity = CreateEntity(std::move(transformation), std::move(projection), std::forward<Rest>(rest)...);
		return Camera(entity);
	}

	Camera PrimaryCamera;

	template<std::derived_from<UpdaterSystem> T, typename... Args>
	T& AddSystem(Args&&... args) requires std::constructible_from<T, Args...>;

	template<std::derived_from<RendererSystem> T, typename... Args>
	T& AddSystem(Args&&... args) requires std::constructible_from<T, Args...>;

	template<std::derived_from<UpdaterSystem> T>
	void EnableSystem();

	template<std::derived_from<UpdaterSystem> T>
	void DisableSystem();

	template<std::derived_from<RendererSystem> T>
	void EnableSystem();

	template<std::derived_from<RendererSystem> T>
	void DisableSystem();

	MaterialHandle CreateMaterial(ShaderHandle type);

	AudioSourceHandle CreateAudioSource();

	   MeshHandle     CreateMesh(const Model& model);
	TextureHandle     CreateTexture(BaseBitmapHandle image, MinFilterMode minFilter = MinFilterMode::LinearMipmapLinear, MagFilterMode magFilter = MagFilterMode::Linear, WrappingMode wrappingMode = WrappingMode::Tiled);
	TextureHandle     CreateCubeMap(BaseBitmapHandle image, MinFilterMode minFilter = MinFilterMode::LinearMipmapLinear, MagFilterMode magFilter = MagFilterMode::Linear, WrappingMode wrappingMode = WrappingMode::ClampedToEdge);
	FrameBufferHandle CreateFrameBuffer(uint32_t width, uint32_t height, const std::vector<AttachmentInfo>& colorAttachments, std::optional<AttachmentInfo> depthAttachments);

	BaseBitmapHandle LoadBitmap(std::span<std::string> facesFileNames, PixelFormat desiredFormat, const std::string& resourceFolderName = "textures");

	BaseBitmapHandle LoadBitmap(const std::string& fileName, PixelFormat desiredFormat, const std::string& resourceFolderName = "textures");

	template<typename Pixel>
	BitmapHandle<Pixel> LoadBitmap(const std::string& fileName, const std::string& resourceFolderName = "textures");

	template<typename Pixel>
	TextureHandle LoadTexture(const std::string& fileName, const std::string& resourceFolderName = "textures", MinFilterMode minFilter = MinFilterMode::LinearMipmapLinear, MagFilterMode magFilter = MagFilterMode::Linear, WrappingMode wrappingMode = WrappingMode::Tiled);

	template<typename Pixel>
	TextureHandle LoadCubeMap(const std::string& left, const std::string& right, const std::string& bottom, const std::string& top, const std::string& back, const std::string& front, const std::string& resourcefolderName = "textures", MinFilterMode minFilter = MinFilterMode::LinearMipmapLinear, MagFilterMode magFilter = MagFilterMode::Linear, WrappingMode wrappingMode = WrappingMode::ClampedToEdge);

	   MeshHandle LoadMesh   (const std::string& fileName, bool smoothNormals = true, const std::string& resourceFolderName = "models");
	   MeshHandle LoadTerrain(const std::string& fileName, const std::string& resourceFolderName = "textures");
	TextureHandle LoadTexture(const std::string& fileName, PixelFormat format, const std::string& resourceFolderName = "textures", MinFilterMode minFilter = MinFilterMode::LinearMipmapLinear, MagFilterMode magFilter = MagFilterMode::Linear, WrappingMode wrappingMode = WrappingMode::Tiled);
	TextureHandle LoadCubeMap(const std::string& left, const std::string& right, const std::string& bottom, const std::string& top, const std::string& back, const std::string& front, PixelFormat format, const std::string& resourceFolderName = "textures", MinFilterMode minFilter = MinFilterMode::LinearMipmapLinear, MagFilterMode magFilter = MagFilterMode::Linear, WrappingMode wrappingMode = WrappingMode::ClampedToEdge);
	 ShaderHandle LoadShader (const std::string& vertexShaderFileName, const std::string& fragmentShaderFileName, const std::string& resourceFolderName = "shaders");
	  SoundHandle LoadSound  (const std::string& fileName, const std::string& resourceFolderName = "sounds");
	  MusicHandle LoadMusic  (const std::string& fileName, const std::string& resourceFolderName = "music");
	   FontHandle LoadFont   (const std::string& fontName, const std::string& resourceFolderName = "fonts");

	MeshHandle CreateQuad();
	MeshHandle CreateCube(); 
	MeshHandle CreatePyramid();
	MeshHandle CreateCone(uint32_t detail = 60);
	MeshHandle CreateCylinder(uint32_t detail = 60);
	MeshHandle CreateSphere(uint32_t detail = 60);
	MeshHandle CreateTerrain(HeightMap heightMap);

	void CalculateTangents(Array<DefaultVertex>& vertices, const std::vector<uint32_t>& indices);

	void  PlayMusic(MusicHandle music, bool loop);
	void  StopMusic();
	void PauseMusic(bool pause);

	void TogglePauseMusic();

	bool IsMusicPaused() const { return m_isMusicPaused; }

	void SetMusicPosition(float seconds) { m_app->GetAudioDevice().SetMusicPosition(seconds); }

	friend class Game;
};

template<typename Pixel>
inline BitmapHandle<Pixel> Scene::LoadBitmap(const std::string& fileName, const std::string& folderName)
{
	BaseBitmapHandle result = LoadBitmap(fileName, PixelFormat::GetFormat<Pixel>(), folderName);
	return std::dynamic_pointer_cast<Bitmap<Pixel>>(result);
}

template<typename Pixel>
inline TextureHandle Scene::LoadTexture(const std::string& fileName, const std::string& folderName, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode)
{
	return LoadTexture(fileName, PixelFormat::GetFormat<Pixel>(), folderName, minFilter, magFilter, wrappingMode);
}

template<typename Pixel>
inline TextureHandle Scene::LoadCubeMap(const std::string& left, const std::string& right, const std::string& bottom, const std::string& top, const std::string& back, const std::string& front, const std::string& resourceFolderName, MinFilterMode minFilter, MagFilterMode magFilter, WrappingMode wrappingMode)
{
	return LoadCubeMap(left, right, bottom, top, back, front, PixelFormat::GetFormat<Pixel>(), resourceFolderName, minFilter, magFilter, wrappingMode);
}

template<typename T, typename ...Args>
inline SceneHandle Scene::OpenScene(Args&&... args) requires std::constructible_from<T, Args...>
{
	SceneHandle result = std::make_shared<T>(args...);
	m_scenes->push(result);
	result->Start(m_app, m_scenes);
	return result;
}

template<std::derived_from<UpdaterSystem> T, typename ...Args>
inline T& Scene::AddSystem(Args&&... args) requires std::constructible_from<T, Args...>
{
	std::shared_ptr<T> system = std::make_shared<T>(args...);
	system->OnStart(*this);
	size_t index = m_updaterSystems.size();
	m_updaterSystems.push_back(system);
	m_updaterSystemIndices[Type::Get<T>()] = index;
	return *system;
}

template<std::derived_from<RendererSystem> T, typename ...Args>
inline T& Scene::AddSystem(Args&&... args) requires std::constructible_from<T, Args...>
{
	std::shared_ptr<T> system = std::make_shared<T>(args...);
	system->OnStart(*this, m_app->GetRenderDevice());
	size_t index = m_rendererSystems.size();
	m_rendererSystems.push_back(system);
	m_rendererSystemIndices[Type::Get<T>()] = index;
	return *system;
}

template<std::derived_from<UpdaterSystem> T>
inline void Scene::EnableSystem()
{
	size_t index = m_updaterSystemIndices[Type::Get<T>()];
	std::shared_ptr<UpdaterSystem> system = m_updaterSystems[index];
	system->IsEnabled = true;
}

template<std::derived_from<UpdaterSystem> T>
inline void Scene::DisableSystem()
{
	size_t index = m_updaterSystemIndices[Type::Get<T>()];
	std::shared_ptr<UpdaterSystem> system = m_updaterSystems[index];
	system->IsEnabled = false;
}

template<std::derived_from<RendererSystem> T>
inline void Scene::EnableSystem()
{
	size_t index = m_rendererSystemIndices[Type::Get<T>()];
	std::shared_ptr<RendererSystem> system = m_rendererSystems[index];
	system->IsEnabled = true;
}

template<std::derived_from<RendererSystem> T>
inline void Scene::DisableSystem()
{
	size_t index = m_rendererSystemIndices[Type::Get<T>()];
	std::shared_ptr<RendererSystem> system = m_rendererSystems[index];
	system->IsEnabled = false;
}
