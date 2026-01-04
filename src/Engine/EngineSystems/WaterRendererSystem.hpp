#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/WaterComponent.hpp"
#include "../EngineSystems/DeferredRendererSystem.hpp"
#include "../Rendering/RenderTarget.hpp"

struct WaterVertex
{
	WaterVertex(const glm::vec2& position) : Position(position) {}

	glm::vec2 Position;

	inline static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(Position)>();
		return result;
	}
};

struct WaterInfo
{
	WaterInfo(const glm::mat4& worldMatrix, const glm::mat4& wvpMatrix) :
		WorldMatrix(worldMatrix), WVPMatrix(wvpMatrix) {}

	glm::mat4 WorldMatrix;
	glm::mat4 WVPMatrix;

	static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(WorldMatrix)>();
		result.AddAttribute<decltype(WVPMatrix)>();
		return result;
	}
};

struct WaterLightInfo
{
	WaterLightInfo(const glm::vec3& direction, const glm::vec3& position, const glm::mat4& projection) :
		Direction(direction), Position(position), Projection(projection) {}

	glm::vec3 Direction;
	glm::vec3 Position;
	glm::mat4 Projection;

	static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(Direction)>();
		result.AddAttribute<decltype(Position)>();
		result.AddAttribute<decltype(Projection)>();
		return result;
	}
};

class WaterRendererSystem : public RendererSystem
{
private:
	const glm::uvec2 m_shadowMapSize;

	std::unordered_map<VertexArrayHandle, Array<MatrixTransformation>> m_shadowMeshQueue;

	VertexArrayHandle m_waterQuad;
	VertexArrayHandle m_screenQuad;

	RenderTargetHandle m_reflectionFrameBuffer;
	RenderTargetHandle m_refractionFrameBuffer;

	RenderTargetHandle m_shadowMapFrameBuffer;

	RenderTargetHandle m_gBuffer;

	ShaderHandle m_blackShader;

	std::unordered_map<TypeInfo*, ShaderHandle> m_lightShaders;

	ShaderHandle m_screenShader;
	ShaderHandle m_shadowMapShader;

	Camera m_altCamera;

	const BufferLayout m_waterInfoLayout;
	const BufferLayout m_lightInfoLayout;
public:
	WaterRendererSystem(const glm::uvec2& shadowMapSize) : m_shadowMapSize(shadowMapSize), m_waterInfoLayout(WaterInfo::GetLayout()), m_lightInfoLayout(WaterLightInfo::GetLayout()) {}

	virtual void OnStart(Scene& scene, RenderDevice& renderDevice)
	{
		Array<WaterVertex> waterVertices =
		{
			WaterVertex(glm::vec2(-1, -1)),
			WaterVertex(glm::vec2( 1, -1)),
			WaterVertex(glm::vec2(-1,  1)),
			WaterVertex(glm::vec2( 1,  1)),
		};

		std::vector<uint32_t> waterIndices =
		{
			0, 1, 2,
			3, 2, 1
		};

		m_waterQuad = renderDevice.CreateMesh(Model(waterVertices, waterIndices))->CreateVertexArray(WaterInfo::GetLayout());

		Array<ScreenVertex> screenVertices =
		{
			glm::vec2(-1, -1), 
			glm::vec2( 1, -1),
			glm::vec2(-1,  1),
			glm::vec2( 1,  1),
		};

		std::vector<uint32_t> screenIndices = 
		{ 
			0, 2, 1,
			1, 2, 3
		};

		m_screenQuad = renderDevice.CreateMesh(Model(screenVertices, screenIndices))->CreateVertexArray(LightInfo::GetLayout());

		m_reflectionFrameBuffer = scene.CreateRenderTarget(1024, 1024,
		{
			AttachmentInfo(InternalImageFormat::RGB8, ImageFormat::RGB, ElementType::UInt8, MinFilterMode::Linear, MagFilterMode::Linear, TextureWrappingMode::ClampedToEdge),
		}, {});

		m_refractionFrameBuffer = scene.CreateRenderTarget(1024, 1024, 
		{
			AttachmentInfo(InternalImageFormat::RGB8, ImageFormat::RGB, ElementType::UInt8, MinFilterMode::Linear, MagFilterMode::Linear, TextureWrappingMode::ClampedToEdge),
		},
		AttachmentInfo(InternalImageFormat::DepthComponent16, ImageFormat::DepthComponent, ElementType::Float32, MinFilterMode::Linear, MagFilterMode::Linear, TextureWrappingMode::ClampedToEdge));

		m_gBuffer = scene.CreateRenderTarget(scene.SelectedGraphicsMode().Width, scene.SelectedGraphicsMode().Height,
		{
			AttachmentInfo(InternalImageFormat::RGB8  , ImageFormat::RGB, ElementType::UInt8  , MinFilterMode::Nearest, MagFilterMode::Nearest, TextureWrappingMode::ClampedToEdge),
			AttachmentInfo(InternalImageFormat::RGB32F, ImageFormat::RGB, ElementType::Float32, MinFilterMode::Nearest, MagFilterMode::Nearest, TextureWrappingMode::ClampedToEdge),
			AttachmentInfo(InternalImageFormat::RGB32F, ImageFormat::RGB, ElementType::Float32, MinFilterMode::Nearest, MagFilterMode::Nearest, TextureWrappingMode::ClampedToEdge),
			AttachmentInfo(InternalImageFormat::R16F  , ImageFormat::R  , ElementType::Float32, MinFilterMode::Nearest, MagFilterMode::Nearest, TextureWrappingMode::ClampedToEdge),
		}, {});

		m_shadowMapFrameBuffer = scene.CreateRenderTarget(m_shadowMapSize.x, m_shadowMapSize.y, {}, AttachmentInfo(InternalImageFormat::DepthComponent32, ImageFormat::DepthComponent, ElementType::Float32, MinFilterMode::Nearest, MagFilterMode::Nearest, TextureWrappingMode::ClampedToEdge));

		m_blackShader = scene.LoadShader("deferred/water/black_VS.glsl", "deferred/water/black_FS.glsl");

		//m_lightShaders[Type::Get<AmbientLight>()    ] = scene.LoadShader("deferred/ambient_VS.glsl", "deferred/ambient_FS.glsl");
		m_lightShaders[TypeInfo::Get<DirectionalLight>()] = scene.LoadShader("deferred/water/directional_VS.glsl", "deferred/water/directional_FS.glsl");
		m_lightShaders[TypeInfo::Get<PointLight>()      ] = scene.LoadShader("deferred/water/point_VS.glsl", "deferred/water/point_FS.glsl");
		m_lightShaders[TypeInfo::Get<SpotLight>()       ] = scene.LoadShader("deferred/water/spot_VS.glsl", "deferred/water/spot_FS.glsl");

		m_screenShader = scene.LoadShader("screen.glsl", "screen_FS.glsl");

		m_shadowMapShader = scene.LoadShader("shadowMapShader_VS.glsl", "shadowMapShader_FS.glsl");

		m_altCamera = scene.CreateCamera(Transformation(), Projection(glm::identity<glm::mat4>()));
	}

	virtual void OnRender(Scene& scene, RenderDevice& renderDevice, RenderContext2D& renderContext2D, RenderTarget& target) override
	{
		target->CopyTo(m_gBuffer, MagFilterMode::Nearest, DepthBuffer);
		m_gBuffer->Use();
		renderDevice.ClearScreen(ColorBuffer);

		scene.DisableSystem<WaterRendererSystem>();

		glm::mat4 viewProjection = scene.PrimaryCamera.GetViewProjection();

		glm::vec3& cameraPosition = scene.PrimaryCamera.GetTransformation().Position;
		glm::quat& cameraRotation = scene.PrimaryCamera.GetTransformation().Rotation;

		bool any = false;

		for(ECS::Entity entity : scene.View<Transformation, WaterComponent>())
		{
			any = true;

			const Transformation& transformation = entity.GetComponent<Transformation>();
			const WaterComponent& waterComponent = entity.GetComponent<WaterComponent>();

			float distance = cameraPosition.y - transformation.Position.y;

			cameraPosition.y -= distance * 2;
			cameraRotation.x = -cameraRotation.x;
			cameraRotation.z = -cameraRotation.z;

			scene.Set("Clipping", true);

			scene.Set("ClippingPlane", glm::vec4(0,  1, 0, -transformation.Position.y));
			scene.Render(renderDevice, renderContext2D, m_reflectionFrameBuffer);

			cameraPosition.y += distance * 2;
			cameraRotation.x = -cameraRotation.x;
			cameraRotation.z = -cameraRotation.z;

			scene.Set("ClippingPlane", glm::vec4(0, -1, 0, transformation.Position.y + 0.1f));
			scene.Render(renderDevice, renderContext2D, m_refractionFrameBuffer);

			scene.Set("Clipping", false);
			renderDevice.Disable(ClipPlane0);

			m_gBuffer->Use();

			ShaderHandle shader = waterComponent.Material->GetType();
			shader->SetTexture("u_reflection", m_reflectionFrameBuffer->GetColorAttachment(0));
			shader->SetTexture("u_refraction", m_refractionFrameBuffer->GetColorAttachment(0));
			shader->SetTexture("u_depthTexture", m_refractionFrameBuffer->GetDepthAttachment());

			shader->TrySet("u_cameraPosition", cameraPosition);

			glm::mat4 worldMatrix = transformation.ToMatrix();
			WaterInfo info(worldMatrix, viewProjection * worldMatrix);

			waterComponent.Material->Use();
			m_waterQuad->Draw(ConstDynamicBufferSlice(&info, 1, m_waterInfoLayout));

			target->Use();
			m_blackShader->Use(nullptr);
			m_waterQuad->Draw(ConstDynamicBufferSlice(&info, 1, m_waterInfoLayout));
		}

		TextureHandle shadowMap = m_shadowMapFrameBuffer->GetDepthAttachment();

		if(any)
		{
			for(ECS::Entity entity : scene.View<Transformation, LightComponent>())
			{
				const Transformation& transformation = entity.GetComponent<Transformation>();
				const LightComponent& lightComponent = entity.GetComponent<LightComponent>();

				LightHandle light = lightComponent.Light;
				glm::vec3 lightDirection = glm::rotate(transformation.GetTransformedRotation(), glm::vec3(0, 0, -1));

				ShaderHandle shader = m_lightShaders[light->GetType()];
				if(!shader)
					continue;

				glm::mat4 altViewProjection = glm::identity<glm::mat4>();
				if(light->ShadowInfo)
				{
					m_altCamera.GetTransformation() = transformation;
					m_altCamera.GetProjection() = light->ShadowInfo->Projection;

					altViewProjection = m_altCamera.GetViewProjection();

					for(ECS::Entity entity : scene.View<Transformation, RenderableMesh>())
					{
						const Transformation& meshTransformation = entity.GetComponent<Transformation>();
						const RenderableMesh& renderableMesh     = entity.GetComponent<RenderableMesh>();

						if(renderableMesh.Emissive)
							continue;

						Buffer<MatrixTransformation>& matrices = m_shadowMeshQueue[renderableMesh.VertexArray];
						matrices.Reserve(scene.MaxEntityCount());
						glm::mat4 worldMatrix = meshTransformation.ToMatrix();
						matrices.Emplace(worldMatrix, altViewProjection * worldMatrix);
					}

					m_shadowMapShader->Use(nullptr);

					m_shadowMapFrameBuffer->Use();
					renderDevice.ClearScreen(DepthBuffer);

					for(auto it = m_shadowMeshQueue.begin(); it != m_shadowMeshQueue.end(); ++it)
					{
						VertexArrayHandle vertexArray = it->first;

						Buffer<MatrixTransformation>& matrices = it->second;

						vertexArray->Draw(matrices);
						matrices.Clear();
					}

					target->Use();
				}

				shader->TrySetTexture("u_normalTexture", m_gBuffer->GetColorAttachment(1));
				shader->TrySetTexture("u_positionTexture", m_gBuffer->GetColorAttachment(2));
				shader->TrySetTexture("u_specularTexture", m_gBuffer->GetColorAttachment(3));

				shader->TrySetTexture("u_shadowMap", shadowMap);

				renderDevice.Disable(RenderFlags::DepthWriting);
				renderDevice.Enable(RenderFlags::Blending);
				renderDevice.SetBlendFunction(BlendFactor::One, BlendFactor::One);

				WaterLightInfo info(lightDirection, transformation.GetTransformedPosition(), altViewProjection);

				shader->TrySet("u_cameraPosition", scene.PrimaryCamera.GetTransformation().Position);
				shader->TrySet("u_shadowMapSize", glm::vec2(shadowMap->Width, shadowMap->Height));

				light->UpdateShader(shader);
				shader->Use(nullptr);
				m_screenQuad->Draw(ConstDynamicBufferSlice(&info, 1, m_lightInfoLayout));

				renderDevice.Disable(RenderFlags::Blending);
				renderDevice.Enable(RenderFlags::DepthWriting);
			}
		}
		renderDevice.Disable(RenderFlags::DepthWriting);
		renderDevice.Enable(RenderFlags::Blending);
		renderDevice.SetBlendFunction(BlendFactor::One, BlendFactor::One);

		target->Use();

		m_screenShader->SetTexture("u_source", m_gBuffer->GetColorAttachment(0));
		m_screenShader->Use(nullptr);
		m_screenQuad->Draw(nullptr);
		
		renderDevice.Disable(RenderFlags::Blending);
		renderDevice.Enable(RenderFlags::DepthWriting);

		scene.EnableSystem<WaterRendererSystem>();
	}
};