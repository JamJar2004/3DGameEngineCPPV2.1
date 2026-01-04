#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/Transformation.hpp"
#include "../EngineComponents/RenderableMesh.hpp"
#include "../EngineComponents/LightComponent.hpp"
#include "../Rendering/RenderStream.hpp"

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

class DeferredRenderContext
{
public:
	explicit DeferredRenderContext(Scene& scene, const glm::uvec2& shadowMapSize = glm::uvec2(1024)) :
		m_lightInfoLayout(LightInfo::GetLayout()),
		m_isRenderingWater(true),
		GBuffer(CreateGBuffer(scene)),
		ShadowMapRenderTarget(CreateShadowRenderTarget(scene, shadowMapSize))
	{
		const std::vector<ScreenVertex> screenVertices
		{
			{ glm::vec2(-1, -1) },
			{ glm::vec2( 1, -1) },
			{ glm::vec2(-1,  1) },
			{ glm::vec2( 1,  1) },
		};

		const std::vector<uint32_t> screenIndices =
		{
			0, 2, 1,
			1, 2, 3
		};

		m_screenQuad = scene.CreateMesh(Model(screenVertices, screenIndices));

		auto shadersFolder = scene.RootAssetFolder.Navigate("shaders");

		auto deferredFolder = shadersFolder.Navigate("deferred");

		LightShaders[TypeInfo::Get<AmbientLight>()    ] = deferredFolder.LoadShader("ambient.glsl");
		LightShaders[TypeInfo::Get<DirectionalLight>()] = deferredFolder.LoadShader("directional.glsl");
		LightShaders[TypeInfo::Get<PointLight>()      ] = deferredFolder.LoadShader("point.glsl");
		LightShaders[TypeInfo::Get<SpotLight>()       ] = deferredFolder.LoadShader("spot.glsl");

		m_shadowMapShader = shadersFolder.LoadShader("shadowMapShader.glsl");

		m_altCamera = scene.CreateCamera(Transformation(), Projection(glm::identity<glm::mat4>()));
	}

	const RenderTargetHandle GBuffer;

	const RenderTargetHandle ShadowMapRenderTarget;

	Camera GetAlternateCamera() const { return m_altCamera; }

	ShaderHandle GetShadowMapShader() const { return m_shadowMapShader; }

	std::unordered_map<TypeInfo*, ShaderHandle> LightShaders;
private:
	static RenderTargetHandle CreateGBuffer(Scene& scene)
	{
		return scene.CreateRenderTarget(scene.SelectedGraphicsMode().Width, scene.SelectedGraphicsMode().Height,
		{
			AttachmentInfo(InternalImageFormat::RGB8  , ImageFormat::RGB, TypeInfo::Get<glm::u8vec3 >(), MinFilterMode::Nearest, MagFilterMode::Nearest, TextureWrappingMode::ClampedToEdge),
			AttachmentInfo(InternalImageFormat::RGB32F, ImageFormat::RGB, TypeInfo::Get<glm::f32vec3>(), MinFilterMode::Nearest, MagFilterMode::Nearest, TextureWrappingMode::ClampedToEdge),
			AttachmentInfo(InternalImageFormat::RGB32F, ImageFormat::RGB, TypeInfo::Get<glm::f32vec3>(), MinFilterMode::Nearest, MagFilterMode::Nearest, TextureWrappingMode::ClampedToEdge),
			AttachmentInfo(InternalImageFormat::R16F  , ImageFormat::R  , TypeInfo::Get<glm::f32vec1>(), MinFilterMode::Nearest, MagFilterMode::Nearest, TextureWrappingMode::ClampedToEdge),
		}, {});
	}

	static RenderTargetHandle CreateShadowRenderTarget(Scene& scene, const glm::uvec2& shadowMapSize)
	{
		return scene.CreateRenderTarget(
			shadowMapSize.x,
			shadowMapSize.y,
			{},
			AttachmentInfo(
				InternalImageFormat::DepthComponent32,
				ImageFormat::DepthComponent,
				TypeInfo::Get<glm::f32vec1>(),
				MinFilterMode::Nearest,
				MagFilterMode::Nearest,
				TextureWrappingMode::ClampedToEdge));
	}

	MeshHandle m_screenQuad;

	ShaderHandle m_shadowMapShader;

	Camera m_altCamera;

	BufferLayout m_lightInfoLayout;

	bool m_isRenderingWater;
};

template<typename TMaterial>
class DeferredRendererSystem final : public RendererSystem
{
public:
	struct RenderInstance final
	{
		RenderInstance(const glm::mat4& modelMatrix, const glm::mat4& mvpMatrix, const TMaterial& material) :
			ModelMatrix(modelMatrix), MVPMatrix(mvpMatrix), Material(material) {}

		glm::mat4 ModelMatrix;
		glm::mat4 MVPMatrix;
		TMaterial Material;
	};

	struct ShadowInstance final
	{
		ShadowInstance(const glm::mat4& modelMatrix, const glm::mat4& mvpMatrix) :
			ModelMatrix(modelMatrix), MVPMatrix(mvpMatrix) {}

		glm::mat4 ModelMatrix;
		glm::mat4 MVPMatrix;
	};

	explicit DeferredRendererSystem(const std::shared_ptr<DeferredRenderContext>& context) : m_context(context),
		m_deferredRenderingTimer("Deferred Render Time") {}

	void OnRender(Scene& scene, RenderDevice& renderDevice, RenderContext2D& renderContext2D, RenderTarget& target) override
	{
		renderDevice.SetFaceCullingMode(FaceCullingMode::Inside);
		renderDevice.Enable(RenderFlags::DepthTest);

		m_context->GBuffer->Clear(ColorBuffer | DepthBuffer);

		ScopeTimer timer(m_deferredRenderingTimer);
		glm::mat4 viewProjection = scene.PrimaryCamera.GetViewProjection();

		for(ECS::Entity entity : scene.RawView<Transformation, RenderableMesh<TMaterial>>())
		{
			const auto& transformation = entity.GetComponent<Transformation>();
			const auto& renderableMesh = entity.GetComponent<RenderableMesh<TMaterial>>();

			if(renderableMesh.ShadowOnly)
			{
				continue;
			}

			glm::mat4 modelMatrix = transformation.ToMatrix();
			glm::mat4 mvpMatrix = viewProjection * modelMatrix;

			m_renderInstances[renderableMesh.Mesh].emplace_back(modelMatrix, mvpMatrix, renderableMesh.Material);

			//Array<MatrixTransformation>* matrices = nullptr;
			//
			//if(!renderableMesh.Emissive)
			//{
			//	matrices = &m_meshQueue[std::make_pair(renderableMesh.VertexArray, renderableMesh.Material)];
			//}
			//else
			//{
			//	matrices = &m_emissiveQueue[std::make_pair(renderableMesh.VertexArray, renderableMesh.Material)];
			//}
			//
			//matrices->Reserve(scene.MaxEntityCount());
			//matrices->Emplace(modelMatrix, mvpMatrix);
		}

		for (auto [mesh, instances] : m_renderInstances)
		{
			bool clipping = scene.TryGet<bool>("Clipping", false);
			if(clipping)
			{
				renderDevice.Enable(ClipPlane0);
				material->GetType()->TrySet("u_clippingPlane", scene.TryGet<glm::vec4>("ClippingPlane"));
			}

			RenderStream<RenderInstance> renderStream(m_renderBuffer, mesh, m_context->GBuffer);
			renderStream.WriteRange(ConstBufferSlice<RenderInstance>(instances.data(), instances.size()));
			renderStream.Flush();

			if(clipping)
			{
				renderDevice.Disable(ClipPlane0);
			}
		}

		//for(auto it = m_meshQueue.begin(); it != m_meshQueue.end(); ++it)
		//{
		//	VertexArrayHandle mesh = it->first.first;
		//	MaterialHandle material = it->first.second;
//
		//	Array<MatrixTransformation>& matrices = it->second;
//
		//	bool clipping = scene.TryGet<bool>("Clipping", false);
		//	if(clipping)
		//	{
		//		renderDevice.Enable(ClipPlane0);
		//		material->GetType()->TrySet("u_clippingPlane", scene.TryGet<glm::vec4>("ClippingPlane"));
		//	}
//
		//	material->Use();
		//	mesh->Draw(matrices);
		//	matrices.Clear();
//
		//	if(clipping)
		//		renderDevice.Disable(ClipPlane0);
		//}

		TextureAtlasHandle shadowMap = m_context->ShadowMapRenderTarget->GetDepthAttachment();

		m_context->GBuffer->CopyTo(target, MagFilterMode::Nearest, DepthBuffer);

		target.Clear(ColorBuffer);

		for(auto [ entity, transformation, lightComponent ] : scene.View<Transformation, LightComponent>())
		{
			const auto& light = lightComponent.Light;
			glm::vec3 lightDirection = glm::rotate(transformation.GetTransformedRotation(), glm::vec3(0, 0, -1));

			auto lightProjection = glm::identity<glm::mat4>();

			auto altViewProjection = glm::identity<glm::mat4>();
			if(light->ShadowInfo)
			{
				auto alternateCamera = m_context->GetAlternateCamera();

				alternateCamera.GetTransformation() = transformation;
				alternateCamera.GetProjection() = light->ShadowInfo->Projection;

				altViewProjection = alternateCamera.GetViewProjection();

				for(auto [ meshEntity, meshTransformation, renderableMesh ] : scene.View<Transformation, RenderableMesh<TMaterial>>())
				{
					if(renderableMesh.Emissive)
					{
						continue;
					}

					glm::mat4 worldMatrix = meshTransformation.ToMatrix();
					m_shadowInstances[renderableMesh.Mesh].emplace_back(worldMatrix, altViewProjection * worldMatrix);
				}

				m_shadowMapShader->Use(nullptr);

				m_context->ShadowMapRenderTarget->Clear(DepthBuffer);

				for (auto [mesh, instances] : m_shadowInstances)
				{
					RenderStream<ShadowInstance> renderStream(m_renderBuffer, mesh, m_context->ShadowMapRenderTarget);
					renderStream.WriteRange(ConstBufferSlice<ShadowInstance>(instances.data(), instances.size()));
					renderStream.Flush();
				}

//
				//m_shadowMapFrameBuffer->Bind();
				//renderDevice.ClearScreen(DepthBuffer);
//
				//for(auto it = m_shadowMeshQueue.begin(); it != m_shadowMeshQueue.end(); ++it)
				//{
				//	VertexArrayHandle vertexArray = it->first;
//
				//	Array<MatrixTransformation>& matrices = it->second;
//
				//	vertexArray->Draw(matrices);
				//	matrices.Clear();
				//}
//
			}

			renderDevice.Disable(DepthWriting);
			renderDevice.Enable(Blending);
			renderDevice.SetBlendFunction(BlendFactor::One, BlendFactor::One);

			ShaderHandle shader = m_context->LightShaders[light->GetType()];
			shader->TrySetTexture("u_albedoTexture", m_context->GBuffer->GetColorAttachment(0));
			shader->TrySetTexture("u_normalTexture", m_context->GBuffer->GetColorAttachment(1));
			shader->TrySetTexture("u_positionTexture", m_context->GBuffer->GetColorAttachment(2));
			shader->TrySetTexture("u_specularTexture", m_context->GBuffer->GetColorAttachment(3));

			shader->TrySetTexture("u_shadowMap", shadowMap);

			LightInfo info(lightDirection, transformation.GetTransformedPosition(), altViewProjection);

			shader->TrySet("u_cameraPosition", scene.PrimaryCamera.GetTransformation().Position);
			shader->TrySet("u_shadowMapSize", glm::vec2(shadowMap->Size));

			light->UpdateShader(shader);
			shader->Use(nullptr);
			m_screenQuad->Draw(ConstDynamicBufferSlice(&info, 1, m_lightInfoLayout));

			renderDevice.Disable(Blending);
			renderDevice.Enable(DepthWriting);
		}

		//target->Bind();

		//for(auto& pair : m_emissiveQueue)
		//{
		//	VertexArrayHandle vertexArray = pair.first.first;
		//	MaterialHandle material = pair.first.second;
//
		//	Array<MatrixTransformation>& matrices = pair.second;
//
		//	bool clipping = scene.TryGet<bool>("Clipping", false);
		//	if(clipping)
		//	{
		//		renderDevice.Enable(ClipPlane0);
		//		material->GetType()->TrySet("u_clippingPlane", scene.TryGet<glm::vec4>("ClippingPlane"));
		//	}
//
		//	material->Use();
		//	vertexArray->Draw(matrices);
		//	matrices.Clear();
//
		//	if(clipping)
		//	{
		//		renderDevice.Disable(ClipPlane0);
		//	}
		//}
	}

private:
	std::shared_ptr<DeferredRenderContext> m_context;

	LocalRenderBufferHandle<RenderInstance> m_renderBuffer;
	LocalRenderBufferHandle<ShadowInstance> m_shadowRenderBuffer;

	std::unordered_map<MeshHandle, std::vector<RenderInstance>> m_renderInstances;
	std::unordered_map<MeshHandle, std::vector<ShadowInstance>> m_shadowInstances;

	Timer m_deferredRenderingTimer;

	//std::unordered_map<DeferredRendererKey, Array<MatrixTransformation>> m_meshQueue;
	//std::unordered_map<DeferredRendererKey, Array<MatrixTransformation>> m_emissiveQueue;
	//std::unordered_map<VertexArrayHandle, Array<MatrixTransformation>> m_shadowMeshQueue;

	//VertexArrayHandle m_screenQuad;

	//std::unordered_map<TypeInfo*, ShaderHandle> m_lightShaders;

	//Camera m_altCamera;

	//RenderTargetHandle m_shadowMapFrameBuffer;

	//BufferLayout m_lightInfoLayout;

	static float CalcRange(const glm::vec3& color, float intensity, const Attenuation& attenuation)
	{
		const float a = attenuation.Exponent;
		const float b = attenuation.Linear;
		const float c = attenuation.Constant - 256 * intensity * glm::compMax(color);

		return (-b + sqrtf(b * b - 4 * a * c)) / (2 * a);
	}
};