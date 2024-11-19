#pragma once

#include <ECS/Component.hpp>
#include <Engine/Rendering/Material.hpp>
#include <Engine/Rendering/Mesh.hpp>

using DeferredRendererKey = std::pair<VertexArrayHandle, MaterialHandle>;

struct MatrixTransformation
{
	MatrixTransformation(const glm::mat4& worldMatrix, const glm::mat4& wvpMatrix) :
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

template<>
struct std::hash<DeferredRendererKey>
{
	size_t operator()(const DeferredRendererKey& renderKey) const
	{
		auto hash1 = hash<VertexArray*>{}(renderKey.first.get());
		auto hash2 = hash<Material*>{}(renderKey.second.get());

		if(hash1 != hash2)
			return hash1 ^ hash2;

		return hash1;
	}
};

struct RenderableMesh : public ECS::Component<RenderableMesh>
{
private:
	static std::unordered_map<MeshHandle, VertexArrayHandle> s_vertexArrayMap;

	static VertexArrayHandle GetVertexArray(MeshHandle mesh);
public:
	RenderableMesh(MeshHandle mesh, MaterialHandle material, bool emissive = false) : VertexArray(GetVertexArray(mesh)), Material(material), Emissive(emissive), ShadowOnly(false) {}

	VertexArrayHandle VertexArray;
	MaterialHandle    Material;

	bool Emissive;
	bool ShadowOnly;
};

//struct EmissiveMesh : public ECS::Component<EmissiveMesh>
//{
//	EmissiveMesh(MeshHandle mesh, MaterialHandle material) : Mesh(mesh), Material(material) {}
//
//	MeshHandle     Mesh;
//	MaterialHandle Material;
//};