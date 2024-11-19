#include "ClickableComponent.hpp"

std::unordered_map<MeshHandle, VertexArrayHandle> ClickableComponent::s_vertexArrayMap;

VertexArrayHandle ClickableComponent::GetVertexArray(MeshHandle mesh)
{
	auto it = s_vertexArrayMap.find(mesh);
	if(it != s_vertexArrayMap.end())
	{
		return it->second;
	}

	return s_vertexArrayMap[mesh] = mesh->CreateVertexArray(EntityIdTransformation::GetLayout());
}
