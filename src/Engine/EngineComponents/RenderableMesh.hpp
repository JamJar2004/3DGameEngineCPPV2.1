#pragma once

#include <ECS/Component.hpp>
#include <Engine/Rendering/Mesh.hpp>

template<ShallowCopyable TMaterial>
class RenderableMesh final : public ECS::Component<RenderableMesh<TMaterial>>
{
public:
	RenderableMesh(const MeshHandle& mesh, const TMaterial& material, bool emissive = false) :
		Mesh(mesh),
		Material(material),
		Emissive(emissive),
		ShadowOnly(false) {}

	MeshHandle Mesh;
	TMaterial  Material;

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