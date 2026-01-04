#pragma once

#include <memory>
#include <vector>

#include "Engine/Core/Buffer.hpp"

class Model
{

public:
	template<HasLayout TVertex>
	Model(const std::vector<TVertex>& vertices, const std::vector<uint32_t>& indices) :
		Vertices(std::make_shared<Buffer<TVertex>>(vertices)),
		Indices(std::make_shared<std::vector<uint32_t>>(indices)),
		m_layout(TVertex::GetLayout())
	{
	}

	const BufferLayout& GetLayout() const { return m_layout; }

	std::shared_ptr<DynamicBuffer>          Vertices;
	std::shared_ptr<std::vector<uint32_t>> Indices;
private:
	BufferLayout m_layout;
};

class Mesh
{
public:
	explicit Mesh(const Model& model) :
		VertexCount(model.Vertices->Count()), IndexCount(model.Indices->size()) {}

	virtual ~Mesh() = default;

	const std::size_t VertexCount;
	const std::size_t  IndexCount;

	friend class RenderDevice;

	template<ShallowCopyable TElement>
	friend class RenderStream;
protected:
	virtual void Draw(size_t instanceCount) = 0;
};

using MeshHandle = std::shared_ptr<Mesh>;