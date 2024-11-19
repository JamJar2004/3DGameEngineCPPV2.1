#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/Transformation.hpp"
#include "../EngineComponents/RenderableMesh.hpp"
#include "../EngineComponents/LightComponent.hpp"
#include "../EngineComponents/SkyboxComponent.hpp"
#include "../EngineComponents/WaterComponent.hpp"

struct LightInfo
{
	LightInfo(const glm::vec3& direction, const glm::vec3& position, const glm::mat4& projection) :
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
//
//struct InstanceInfo
//{
//	InstanceInfo(const glm::mat4& wvpMatrix) : WVPMatrix(wvpMatrix) {}
//
//	glm::mat4 WVPMatrix;
//
//	inline static BufferLayout GetLayout()
//	{
//		BufferLayout result;
//		result.AddAttribute<decltype(WVPMatrix)>();
//		return result;
//	}
//};

class DeferredRendererSystem : public RendererSystem
{
private:
	Timer m_defferedRenderingTimer;

	const glm::uvec2 m_shadowMapSize;

	std::unordered_map<DeferredRendererKey, Array<MatrixTransformation>> m_meshQueue;
	std::unordered_map<DeferredRendererKey, Array<MatrixTransformation>> m_emissiveQueue;
	std::unordered_map<VertexArrayHandle, Array<MatrixTransformation>> m_shadowMeshQueue;

	FrameBufferHandle m_gBuffer;

	VertexArrayHandle m_screenQuad;

	std::unordered_map<Type*, ShaderHandle> m_lightShaders;

	ShaderHandle m_shadowMapShader;

	Scene* m_scene;

	Camera m_altCamera;

	FrameBufferHandle m_shadowMapFrameBuffer;
	
	BufferLayout m_lightInfoLayout;
	
	bool m_isRenderingWater;

	static float CalcRange(const glm::vec3& color, float intensity, const Attenuation& attenuation)
	{
		float a = attenuation.Exponent;
		float b = attenuation.Linear;
		float c = attenuation.Constant - 256 * intensity * glm::compMax(color);

		return (-b + sqrtf(b * b - 4 * a * c)) / (2 * a);
	}
public:
	DeferredRendererSystem(const glm::uvec2& shadowMapSize = glm::uvec2(1024)) : 
		m_shadowMapSize(shadowMapSize), m_scene(nullptr), 
		m_lightInfoLayout(LightInfo::GetLayout()),
		m_isRenderingWater(true),
		m_defferedRenderingTimer("Deffered Render Time")
	{

	}

	virtual void OnStart(Scene& scene, RenderDevice& renderDevice) override
	{
		m_scene = &scene;

		m_gBuffer = scene.CreateFrameBuffer(scene.GraphicsMode().Width, scene.GraphicsMode().Height,
		{
			AttachmentInfo(InternalFormat::RGB8  , Format::RGB, ElementType::UInt8  , MinFilterMode::Nearest, MagFilterMode::Nearest, WrappingMode::ClampedToEdge),
			AttachmentInfo(InternalFormat::RGB32F, Format::RGB, ElementType::Float32, MinFilterMode::Nearest, MagFilterMode::Nearest, WrappingMode::ClampedToEdge),
			AttachmentInfo(InternalFormat::RGB32F, Format::RGB, ElementType::Float32, MinFilterMode::Nearest, MagFilterMode::Nearest, WrappingMode::ClampedToEdge),
			AttachmentInfo(InternalFormat::R16F  , Format::R  , ElementType::Float32, MinFilterMode::Nearest, MagFilterMode::Nearest, WrappingMode::ClampedToEdge),
		}, {});

		m_shadowMapFrameBuffer = scene.CreateFrameBuffer(m_shadowMapSize.x, m_shadowMapSize.y, {}, AttachmentInfo(InternalFormat::DepthComponent32, Format::DepthComponent, ElementType::Float32, MinFilterMode::Nearest, MagFilterMode::Nearest, WrappingMode::ClampedToEdge));

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

		m_lightShaders[Type::Get<AmbientLight>()    ] = scene.LoadShader("deferred/ambient_VS.glsl", "deferred/ambient_FS.glsl");
		m_lightShaders[Type::Get<DirectionalLight>()] = scene.LoadShader("deferred/directional_VS.glsl", "deferred/directional_FS.glsl");
		m_lightShaders[Type::Get<PointLight>()      ] = scene.LoadShader("deferred/point_VS.glsl", "deferred/point_FS.glsl");
		m_lightShaders[Type::Get<SpotLight>()       ] = scene.LoadShader("deferred/spot_VS.glsl", "deferred/spot_FS.glsl");

		/*m_ambientLightShader->GetMaterialField("Color").SetDefaultValue(glm::vec3(1));
		m_ambientLightShader->GetMaterialField("Intensity").SetDefaultValue(1.0f);

		m_directionalLightShader->GetMaterialField("Color").SetDefaultValue(glm::vec3(1));
		m_directionalLightShader->GetMaterialField("Intensity").SetDefaultValue(1.0f);

		m_pointLightShader->GetMaterialField("Color").SetDefaultValue(glm::vec3(1));
		m_pointLightShader->GetMaterialField("Intensity").SetDefaultValue(1.0f);
		m_pointLightShader->GetMaterialField("Constant").SetDefaultValue(0.0f);
		m_pointLightShader->GetMaterialField("Linear").SetDefaultValue(0.0f);
		m_pointLightShader->GetMaterialField("Exponent").SetDefaultValue(1.0f);
		m_pointLightShader->GetMaterialField("Range").SetDefaultValue(CalcRange(glm::vec3(1), 1.0f, Attenuation(0.0f, 0.0f, 1.0f)));

		m_spotLightShader->GetMaterialField("Color").SetDefaultValue(glm::vec3(1));
		m_spotLightShader->GetMaterialField("Intensity").SetDefaultValue(1.0f);
		m_spotLightShader->GetMaterialField("Constant").SetDefaultValue(0.0f);
		m_spotLightShader->GetMaterialField("Linear").SetDefaultValue(0.0f);
		m_spotLightShader->GetMaterialField("Exponent").SetDefaultValue(1.0f);
		m_spotLightShader->GetMaterialField("Range").SetDefaultValue(CalcRange(glm::vec3(1), 1.0f, Attenuation(0.0f, 0.0f, 1.0f)));
		m_spotLightShader->GetMaterialField("Cutoff").SetDefaultValue(0.4f);*/

		m_shadowMapShader = scene.LoadShader("shadowMapShader_VS.glsl", "shadowMapShader_FS.glsl");

		m_altCamera = scene.CreateCamera(Transformation(), Projection(glm::identity<glm::mat4>()));
	}

	virtual void OnRender(Scene& scene, RenderDevice& renderDevice, RenderContext2D& renderContext2D, FrameBufferHandle target) override
	{
		{
			m_gBuffer->Bind();

			renderDevice.SetFaceCullingMode(FaceCullingMode::Inside);
			renderDevice.Enable(RenderFlags::DepthTest);

			renderDevice.ClearScreen(ColorBuffer | DepthBuffer);

		
			ScopeTimer timer(m_defferedRenderingTimer);
			glm::mat4 viewProjection = scene.PrimaryCamera.GetViewProjection();
			for(ECS::Entity entity : scene.View<Transformation, RenderableMesh>())
			{
				const Transformation& transformation = entity.GetComponent<Transformation>();
				const RenderableMesh& renderableMesh = entity.GetComponent<RenderableMesh>();

				if(renderableMesh.ShadowOnly)
				{
					continue;
				}

				glm::mat4 worldMatrix = transformation.ToMatrix();
				glm::mat4 wvpMatrix = viewProjection * worldMatrix;

				Array<MatrixTransformation>* matrices = nullptr;

				if(!renderableMesh.Emissive)
				{
					matrices = &m_meshQueue[std::make_pair(renderableMesh.VertexArray, renderableMesh.Material)];
				}
				else
				{
					matrices = &m_emissiveQueue[std::make_pair(renderableMesh.VertexArray, renderableMesh.Material)];
				}

				matrices->Reserve(scene.MaxEntityCount());
				matrices->Emplace(worldMatrix, wvpMatrix);
			}

			for(auto it = m_meshQueue.begin(); it != m_meshQueue.end(); ++it)
			{
				VertexArrayHandle mesh = it->first.first;
				MaterialHandle material = it->first.second;

				Array<MatrixTransformation>& matrices = it->second;

				bool clipping = scene.TryGet<bool>("Clipping", false);
				if(clipping)
				{
					renderDevice.Enable(ClipPlane0);
					material->GetType()->TrySet("u_clippingPlane", scene.TryGet<glm::vec4>("ClippingPlane"));
				}

				material->Use();
				mesh->Draw(matrices);
				matrices.Clear();

				if(clipping)
					renderDevice.Disable(ClipPlane0);
			}

			TextureHandle shadowMap = m_shadowMapFrameBuffer->GetDepthAttachment();

			m_gBuffer->CopyTo(target, MagFilterMode::Nearest, DepthBuffer);

			target->Bind();
			renderDevice.ClearScreen(ColorBuffer);

			for(ECS::Entity entity : scene.View<Transformation, LightComponent>())
			{
				const Transformation& transformation = entity.GetComponent<Transformation>();
				const LightComponent& lightComponent = entity.GetComponent<LightComponent>();

				LightHandle light = lightComponent.Light;
				glm::vec3 lightDirection = glm::rotate(transformation.GetTransformedRotation(), glm::vec3(0, 0, -1));

				glm::mat4 lightProjection = glm::identity<glm::mat4>();

				glm::mat4 altViewProjection = glm::identity<glm::mat4>();
				if(light->ShadowInfo)
				{
					m_altCamera.GetTransformation() = transformation;
					m_altCamera.GetProjection() = light->ShadowInfo->Projection;

					altViewProjection = m_altCamera.GetViewProjection();

					for(ECS::Entity entity : scene.View<Transformation, RenderableMesh>())
					{
						const Transformation& meshTransformation = entity.GetComponent<Transformation>();
						const RenderableMesh& renderableMesh = entity.GetComponent<RenderableMesh>();

						if(renderableMesh.Emissive)
						{
							continue;
						}

						Array<MatrixTransformation>& matrices = m_shadowMeshQueue[renderableMesh.VertexArray];
						matrices.Reserve(scene.MaxEntityCount());
						glm::mat4 worldMatrix = meshTransformation.ToMatrix();
						matrices.Emplace(worldMatrix, altViewProjection * worldMatrix);
					}

					m_shadowMapShader->Use(nullptr);

					m_shadowMapFrameBuffer->Bind();
					renderDevice.ClearScreen(DepthBuffer);

					for(auto it = m_shadowMeshQueue.begin(); it != m_shadowMeshQueue.end(); ++it)
					{
						VertexArrayHandle vertexArray = it->first;

						Array<MatrixTransformation>& matrices = it->second;

						vertexArray->Draw(matrices);
						matrices.Clear();
					}

					target->Bind();
				}

				renderDevice.Disable(RenderFlags::DepthWriting);
				renderDevice.Enable(RenderFlags::Blending);
				renderDevice.SetBlendFunction(BlendFactor::One, BlendFactor::One);

				ShaderHandle shader = m_lightShaders[light->GetType()];
				shader->TrySetTexture("u_albedoTexture", m_gBuffer->GetColorAttachment(0));
				shader->TrySetTexture("u_normalTexture", m_gBuffer->GetColorAttachment(1));
				shader->TrySetTexture("u_positionTexture", m_gBuffer->GetColorAttachment(2));
				shader->TrySetTexture("u_specularTexture", m_gBuffer->GetColorAttachment(3));

				shader->TrySetTexture("u_shadowMap", shadowMap);

				LightInfo info(lightDirection, transformation.GetTransformedPosition(), altViewProjection);

				shader->TrySet("u_cameraPosition", scene.PrimaryCamera.GetTransformation().Position);
				shader->TrySet("u_shadowMapSize", glm::vec2(shadowMap->Size));

				light->UpdateShader(shader);
				shader->Use(nullptr);
				m_screenQuad->Draw(BaseArraySlice(&info, 1, m_lightInfoLayout));

				renderDevice.Disable(RenderFlags::Blending);
				renderDevice.Enable(RenderFlags::DepthWriting);
			}


			target->Bind();

			for(auto it = m_emissiveQueue.begin(); it != m_emissiveQueue.end(); ++it)
			{
				VertexArrayHandle vertexArray = it->first.first;
				MaterialHandle material = it->first.second;

				Array<MatrixTransformation>& matrices = it->second;

				bool clipping = scene.TryGet<bool>("Clipping", false);
				if(clipping)
				{
					renderDevice.Enable(ClipPlane0);
					material->GetType()->TrySet("u_clippingPlane", scene.TryGet<glm::vec4>("ClippingPlane"));
				}

				material->Use();
				vertexArray->Draw(matrices);
				matrices.Clear();

				if(clipping)
					renderDevice.Disable(ClipPlane0);
			}
		}
	}
};