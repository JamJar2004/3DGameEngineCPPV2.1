#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/Transformation.hpp"
#include "../EngineComponents/RenderableMesh.hpp"
#include "../EngineComponents/LightComponent.hpp"
#include "../EngineComponents/SkyboxComponent.hpp"

struct SkyboxVertex
{
	SkyboxVertex(const glm::vec3& position) : Position(position) {}

	glm::vec3 Position;

	inline static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(Position)>();
		return result;
	}
};

struct SkyboxInfo
{
	SkyboxInfo(const glm::mat4& wvpMatrix) : WVPMatrix(wvpMatrix) {}

	glm::mat4 WVPMatrix;
	
	inline static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(WVPMatrix)>();
		return result;
	}
};

inline static MeshHandle CreateCube(Scene& scene)
{
	Array<SkyboxVertex> vertices =
	{
		SkyboxVertex(glm::vec3(-1, -1,  1)),
		SkyboxVertex(glm::vec3(-1,  1,  1)),
		SkyboxVertex(glm::vec3(-1, -1, -1)),
		SkyboxVertex(glm::vec3(-1,  1, -1)),

		SkyboxVertex(glm::vec3(1, -1,  1)),
		SkyboxVertex(glm::vec3(1, -1, -1)),
		SkyboxVertex(glm::vec3(1,  1,  1)),
		SkyboxVertex(glm::vec3(1,  1, -1)),

		SkyboxVertex(glm::vec3(-1, -1,   1)),
		SkyboxVertex(glm::vec3(-1, -1,  -1)),
		SkyboxVertex(glm::vec3( 1, -1,   1)),
		SkyboxVertex(glm::vec3( 1, -1,  -1)),

		SkyboxVertex(glm::vec3(-1, 1,  1)),
		SkyboxVertex(glm::vec3( 1, 1,  1)),
		SkyboxVertex(glm::vec3(-1, 1, -1)),
		SkyboxVertex(glm::vec3( 1, 1, -1)),

		SkyboxVertex(glm::vec3(-1, -1, 1)),
		SkyboxVertex(glm::vec3( 1, -1, 1)),
		SkyboxVertex(glm::vec3(-1,  1, 1)),
		SkyboxVertex(glm::vec3( 1,  1, 1)),

		SkyboxVertex(glm::vec3(-1, -1, -1)),
		SkyboxVertex(glm::vec3(-1,  1, -1)),
		SkyboxVertex(glm::vec3( 1, -1, -1)),
		SkyboxVertex(glm::vec3( 1,  1, -1))
	};

	size_t faceIndices[] =
	{
		0, 2, 3,
		3, 1, 0,
	};

	std::vector<uint32_t> indices;
	indices.reserve(36);

	for(size_t i = 0; i < 6; i++)
	{
		for(size_t j = 0; j < 6; j++)
			indices.push_back(uint32_t(faceIndices[j] + i * 4));
	}

	return scene.CreateMesh(Model(vertices, indices));
}



class SkyboxRendererSystem : public RendererSystem
{
private:
	VertexArrayHandle m_cube;

	std::unordered_map<MaterialHandle, Array<SkyboxInfo>> m_renderQueue;
public:
	virtual void OnStart(Scene& scene, RenderDevice& renderDevice) override
	{
		m_cube = CreateCube(scene)->CreateVertexArray(SkyboxInfo::GetLayout());
	}

	virtual void OnRender(Scene& scene, RenderDevice& renderDevice, RenderContext2D& renderContext2D, FrameBufferHandle target) override
	{
		glm::mat4 viewProjection = scene.PrimaryCamera.GetViewProjection();

		renderDevice.SetFaceCullingMode(FaceCullingMode::Outside);
		
		for(ECS::Entity entity : scene.View<Transformation, SkyboxComponent>())
		{
			const Transformation&  transformation  = entity.GetComponent<Transformation>();
			const SkyboxComponent& skyBoxComponent = entity.GetComponent<SkyboxComponent>();

			m_renderQueue[skyBoxComponent.Material].Emplace(viewProjection * transformation.ToMatrix());
		}

		for(auto it = m_renderQueue.begin(); it != m_renderQueue.end(); ++it)
		{
			MaterialHandle material = it->first;

			Array<SkyboxInfo>& matrices = it->second;

			material->Use();
			m_cube->Draw(matrices);
			matrices.Clear();
		}

		renderDevice.SetFaceCullingMode(FaceCullingMode::Inside);
	}
};