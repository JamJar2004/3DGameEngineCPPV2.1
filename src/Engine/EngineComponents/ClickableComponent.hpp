#pragma once

#include <ECS/Component.hpp>
#include <Engine/Rendering/Mesh.hpp>

class EntityIdTransformation
{
public:
	EntityIdTransformation(const glm::mat4& wvpMatrix, uint32_t id) : WVPMatrix(wvpMatrix), Id(id) {}

	glm::mat4 WVPMatrix;
	uint32_t Id;

	static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(WVPMatrix)>();
		result.AddAttribute<decltype(Id)>();
		return result;
	}
};

class ClickableComponent : public ECS::Component<ClickableComponent>
{
public:
	explicit ClickableComponent(const MeshHandle& mesh) : Mesh(mesh) {}

	MeshHandle Mesh;
};