#include "RenderableMesh.hpp"

std::unordered_map<MeshHandle, VertexArrayHandle> RenderableMesh::s_vertexArrayMap;

VertexArrayHandle RenderableMesh::GetVertexArray(MeshHandle mesh)
{
	auto it = s_vertexArrayMap.find(mesh);
	if(it != s_vertexArrayMap.end())
	{
		return it->second;
	}

	return s_vertexArrayMap[mesh] = mesh->CreateVertexArray(MatrixTransformation::GetLayout());
}
