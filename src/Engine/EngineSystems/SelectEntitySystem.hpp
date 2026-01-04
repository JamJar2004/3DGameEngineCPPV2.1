#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/RenderableMesh.hpp"
#include "../EngineComponents/ClickableComponent.hpp"

class EntityIdRendererSystem : public RendererSystem
{
private:
	RenderTargetHandle m_entityIdBuffer;

	std::unordered_map<VertexArrayHandle, Buffer<EntityIdTransformation>> m_meshQueue;
	std::unordered_map<VertexArrayHandle, Buffer<MatrixTransformation>> m_nullEntityQueue;

	ShaderHandle m_entityIdShader;
	ShaderHandle m_nullEntityIdShader;
public:
	EntityIdRendererSystem() {}

	TextureHandle GetEntityIdTextureMap() const { return m_entityIdBuffer->GetColorAttachment(0); }

	BitmapHandle<uint32_t> EntityMap;

	virtual void OnStart(Scene& scene, RenderDevice& renderDevice) override
	{
		m_entityIdBuffer = scene.CreateRenderTarget(renderDevice.ScreenBuffer->Width, renderDevice.ScreenBuffer->Height,
		{
			AttachmentInfo(InternalImageFormat::R32, ImageFormat::R, ElementType::UInt32, MinFilterMode::Nearest, MagFilterMode::Nearest, TextureWrappingMode::ClampedToEdge),
		},
		{});

		m_entityIdShader = scene.LoadShader("entityIdMapGenerator_VS.glsl", "entityIdMapGenerator_FS.glsl");
	
		m_nullEntityIdShader = scene.LoadShader("entityIdNullGenerator_VS.glsl", "entityIdNullGenerator_FS.glsl");

		m_entityIdBuffer->Use();
		uint32_t nullValue = ECS::Entity::Null.GetIndex();
		renderDevice.ClearColorBuffer(PixelFormat::GetFormat<uint32_t>(), &nullValue);
		renderDevice.ScreenBuffer->Use();

		EntityMap = std::make_shared<Bitmap<uint32_t>>(renderDevice.ScreenBuffer->Width, renderDevice.ScreenBuffer->Height);
		EntityMap->ZeroPixels();
	}

	virtual void OnRender(Scene& scene, RenderDevice& renderDevice, RenderContext2D& renderContext2D, RenderTarget& target) override
	{
		m_entityIdBuffer->Use();

		renderDevice.SetFaceCullingMode(FaceCullingMode::Inside);
		renderDevice.Enable(RenderFlags::DepthTest);

		uint32_t nullValue = ECS::Entity::Null.GetIndex();
		renderDevice.ClearColorBuffer(EntityMap->Format, &nullValue);

		renderDevice.ClearScreen(DepthBuffer);

		glm::mat4 viewProjection = scene.PrimaryCamera.GetViewProjection();
		for(ECS::Entity entity : scene.View<Transformation, RenderableMesh>())
		{
			if(entity.ContainsComponent<ClickableComponent>())
			{
				continue;
			}

			const Transformation& transformation = entity.GetComponent<Transformation>();
			const RenderableMesh& renderableMesh = entity.GetComponent<RenderableMesh>();

			glm::mat4 worldMatrix = transformation.ToMatrix();
			glm::mat4 wvpMatrix = viewProjection * worldMatrix;

			Buffer<MatrixTransformation>& matrices = m_nullEntityQueue[renderableMesh.VertexArray];
			matrices.Reserve(scene.MaxEntityCount());
			matrices.Emplace(worldMatrix, wvpMatrix);
		}
		
		for(ECS::Entity entity : scene.View<Transformation, ClickableComponent>())
		{
			const Transformation&     transformation     = entity.GetComponent<Transformation>();
			const ClickableComponent& clickableComponent = entity.GetComponent<ClickableComponent>();

			glm::mat4 worldMatrix = transformation.ToMatrix();
			glm::mat4 wvpMatrix = viewProjection * worldMatrix;

			Buffer<EntityIdTransformation>& matrices = m_meshQueue[clickableComponent.VertexArray];
			matrices.Reserve(scene.MaxEntityCount());
			matrices.Emplace(wvpMatrix, entity.GetIndex());
		}

		m_nullEntityIdShader->Use(nullptr);

		for(auto it = m_nullEntityQueue.begin(); it != m_nullEntityQueue.end(); ++it)
		{
			VertexArrayHandle mesh = it->first;

			Buffer<MatrixTransformation>& matrices = it->second;

			mesh->Draw(matrices);
			matrices.Clear();
		}

		m_entityIdShader->Use(nullptr);

		for(auto it = m_meshQueue.begin(); it != m_meshQueue.end(); ++it)
		{
			VertexArrayHandle vertexArray = it->first;
			Buffer<EntityIdTransformation>& matrices = it->second;
			
			vertexArray->Draw(matrices);
			matrices.Clear();
		}

		m_entityIdBuffer->GetColorAttachment(0)->CopyTo(EntityMap);
	}
};

class MousePickSystem : public UpdaterSystem
{
private:
	EntityIdRendererSystem& m_system;
public:
	MousePickSystem(EntityIdRendererSystem& system) : m_system(system) {}

	ECS::EntityEvent<> OnEntityHover;

	virtual void OnStart(Scene& scene) override
	{

	}

	virtual void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) override
	{
		glm::vec2 mousePosition = (mouse.GetPosition() * 0.5f + 0.5f) * glm::vec2(m_system.EntityMap->Size());
		glm::uvec2 pixelLocation = glm::uvec2(mousePosition);
			
		uint32_t entityId = m_system.EntityMap->GetPixel(pixelLocation);
		if(entityId == 0)
		{
			return;
		}

		ECS::Entity entity = scene.GetEntity(entityId);
		if(entity.IsValid())
		{
			OnEntityHover(entity);
		}
	}
};