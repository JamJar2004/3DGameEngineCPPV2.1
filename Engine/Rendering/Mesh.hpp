#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <vector>

#include "Common.hpp"
#include "Material.hpp"

class BufferAttribute
{
private:
	size_t GetTypeSize() const
	{
		switch(Type)
		{
			case ElementType::UInt8:
				return sizeof(uint8_t);
			case ElementType::UInt16:
				return sizeof(uint16_t);
			case ElementType::UInt32:
				return sizeof(uint32_t);
			case ElementType::UInt64:
				return sizeof(uint64_t);
			case ElementType::SInt8:
				return sizeof(int8_t);
			case ElementType::SInt16:
				return sizeof(int16_t);
			case ElementType::SInt32:
				return sizeof(int32_t);
			case ElementType::SInt64:
				return sizeof(int64_t);
			case ElementType::Float32:
				return sizeof(float);
			case ElementType::Float64:
				return sizeof(double);
		}

		return 0;
	}
public:
	BufferAttribute(ElementType type, size_t count, size_t offset) :
		Type(type), Count(count), Offset(offset) {}

	ElementType Type;
	size_t      Count;
	size_t      Offset;

	size_t SizeInBytes() const { return Count * GetTypeSize(); }
};

class BufferLayout
{
private:
	std::vector<BufferAttribute> m_attributes;
	size_t m_stride;
public:
	BufferLayout() : m_stride(0) {}

	void AddAttribute(const BufferAttribute& element)
	{
		m_attributes.push_back(element);
		m_stride += element.SizeInBytes();
	}

	size_t GetStride() const { return m_stride; }

	template<typename T>
	void AddAttribute() {}

	template<>
	void AddAttribute<uint8_t>() { AddAttribute(BufferAttribute(ElementType::UInt8, 1, m_stride)); }

	template<>
	void AddAttribute<int8_t>() { AddAttribute(BufferAttribute(ElementType::SInt8, 1, m_stride)); }

	template<>
	void AddAttribute<uint32_t>() { AddAttribute(BufferAttribute(ElementType::UInt32, 1, m_stride)); }

	template<>
	void AddAttribute<int32_t>() { AddAttribute(BufferAttribute(ElementType::SInt32, 1, m_stride)); }

	template<>
	void AddAttribute<glm::ivec2>() { AddAttribute(BufferAttribute(ElementType::SInt32, 2, m_stride)); }

	template<>
	void AddAttribute<glm::ivec3>() { AddAttribute(BufferAttribute(ElementType::SInt32, 3, m_stride)); }

	template<>
	void AddAttribute<glm::ivec4>() { AddAttribute(BufferAttribute(ElementType::SInt32, 4, m_stride)); }

	template<>
	void AddAttribute<float>() { AddAttribute(BufferAttribute(ElementType::Float32, 1, m_stride)); }

	template<>
	void AddAttribute<glm::vec2>() { AddAttribute(BufferAttribute(ElementType::Float32, 2, m_stride)); }

	template<>
	void AddAttribute<glm::vec3>() { AddAttribute(BufferAttribute(ElementType::Float32, 3, m_stride)); }

	template<>
	void AddAttribute<glm::vec4>() { AddAttribute(BufferAttribute(ElementType::Float32, 4, m_stride)); }

	template<>
	void AddAttribute<glm::mat2>() { AddAttribute(BufferAttribute(ElementType::Float32, 2 * 2, m_stride)); }

	template<>
	void AddAttribute<glm::mat3>() { AddAttribute(BufferAttribute(ElementType::Float32, 3 * 3, m_stride)); }

	template<>
	void AddAttribute<glm::mat4>() { AddAttribute(BufferAttribute(ElementType::Float32, 4 * 4, m_stride)); }

	template<>
	void AddAttribute<double>() { AddAttribute(BufferAttribute(ElementType::Float64, 1, m_stride)); }

	template<>
	void AddAttribute<glm::dvec2>() { AddAttribute(BufferAttribute(ElementType::Float64, 2, m_stride)); }

	template<>
	void AddAttribute<glm::dvec3>() { AddAttribute(BufferAttribute(ElementType::Float64, 3, m_stride)); }

	template<>
	void AddAttribute<glm::dvec4>() { AddAttribute(BufferAttribute(ElementType::Float64, 4, m_stride)); }

	template<>
	void AddAttribute<glm::dmat2>() { AddAttribute(BufferAttribute(ElementType::Float64, 2 * 2, m_stride)); }

	template<>
	void AddAttribute<glm::dmat3>() { AddAttribute(BufferAttribute(ElementType::Float64, 3 * 3, m_stride)); }

	template<>
	void AddAttribute<glm::dmat4>() { AddAttribute(BufferAttribute(ElementType::Float64, 4 * 4, m_stride)); }

	auto begin() const { return m_attributes.begin(); }
	auto end()   const { return m_attributes.end();   }
};

template<typename T>
concept HasLayout = requires
{
	{ T::GetLayout() } -> std::same_as<BufferLayout>;
};

class BaseArray
{
private:
	const BufferLayout m_layout;
public:
	BaseArray(const BufferLayout& layout) : m_layout(layout) {}

	const BufferLayout& GetLayout() const { return m_layout; }

	virtual const void* Data() const = 0;
	virtual size_t      Count() const = 0;
};

template<HasLayout T>
class Array : public BaseArray
{
private:
	std::vector<T> m_elements;
public:
	Array() : BaseArray(T::GetLayout()) {}

	Array(std::initializer_list<T> vertices) : BaseArray(T::GetLayout()), m_elements(vertices) {}

	      T& operator[](size_t index)       { return m_elements[index]; }
	const T& operator[](size_t index) const { return m_elements[index]; }

	void Reserve(size_t capacity) { m_elements.reserve(capacity); }

	void Add(const T& vertex) { m_elements.push_back(vertex); }

	template<typename... Args>
	void Emplace(Args&&... args) requires std::constructible_from<T, Args...> { m_elements.emplace_back(args...); }

	void Clear() { m_elements.clear(); }

	virtual const void* Data() const override { return m_elements.data(); }

	virtual size_t Count() const override { return m_elements.size(); }
};

//template<HasLayout T>
//class ArraySlice
//{
//private:
//	T* const     m_address;
//	const size_t m_count;
//public:
//	ArraySlice(Array<T>& array) : ArraySlice(array, 0, array.Count()) {}
//
//	ArraySlice(Array<T>& array, size_t offset, size_t count) :
//		m_address(array.Data() + offset), m_count(count) {}
//
//	size_t Count() const { return m_count; }
//
//	T& operator[](size_t index) const { return m_address[index]; }
//
//	T* Data() const { return m_address; }
//};

template<HasLayout T>
class ArraySlice
{
private:
	const T* const m_address;
	const size_t   m_count;

	const BufferLayout* m_layout;
public:
	ArraySlice(std::nullptr_t) : m_address(nullptr), m_count(0), m_layout(nullptr) {}

	ArraySlice(const T* address, size_t count, const BufferLayout& layout) :
		m_address(address), m_count(count), m_layout(&layout) {}

	ArraySlice(const Array<T>& array) : ArraySlice(array, 0, array.Count()) {}

	ArraySlice(const Array<T>& array, size_t offset, size_t count) :
		m_address((const T*)array.Data() + offset), m_count(count), m_layout(&array.GetLayout()) {}

	operator bool() const { return m_address; }

	size_t Count() const { return m_count; }

	const T& operator[](size_t index) const { return m_address[index]; }

	const T* Data() const { return m_address; }

	const BufferLayout& GetLayout() const { return *m_layout; }

	ArraySlice<T> Slice(size_t index, size_t count) const { return ArraySlice<T>(m_address + index, count, *m_layout); }
};

class BaseArraySlice
{
private:
	const void* const m_address;
	const size_t      m_count;

	const BufferLayout* m_layout;
public:
	BaseArraySlice(std::nullptr_t) : m_address(nullptr), m_count(0), m_layout(nullptr) {}

	BaseArraySlice(const void* address, size_t count, const BufferLayout& layout) :
		m_address(address), m_count(count), m_layout(&layout) {}

	BaseArraySlice(const BaseArray& array) : BaseArraySlice(array, 0, array.Count()) {}

	BaseArraySlice(const BaseArray& array, size_t offset, size_t count) :
		m_address((uint8_t*)(array.Data()) + offset * array.GetLayout().GetStride()), 
		m_count(count),
		m_layout(&array.GetLayout()) {}

	template<HasLayout T>
	BaseArraySlice(ArraySlice<T> array) :
		m_address(array.Data()),
		m_count(array.Count()),
		m_layout(&array.GetLayout()) {}

	operator bool() const { return m_address; }

	size_t Count() const { return m_count; }

	const void* Data() const { return m_address; }

	const BufferLayout& GetLayout() const { return *m_layout; }

	BaseArraySlice Slice(size_t index, size_t count) const { return BaseArraySlice((uint8_t*)m_address + index * m_layout->GetStride(), count, *m_layout); }
};

class Model
{
public:
	template<std::derived_from<BaseArray> T>
	Model(const T& vertices, const std::vector<uint32_t>& indices) :
		Vertices(std::make_shared<T>(vertices)), Indices(std::make_shared<std::vector<uint32_t>>(indices)) {}

	std::shared_ptr<BaseArray>             Vertices;
	std::shared_ptr<std::vector<uint32_t>> Indices;
};

class VertexArray
{
private:
public:
	VertexArray(const BufferLayout& instanceLayout) : InstanceLayout(instanceLayout) {}

	const BufferLayout InstanceLayout;

	virtual void Draw(BaseArraySlice instanceElements) = 0;
};

using VertexArrayHandle = std::shared_ptr<VertexArray>;

class Mesh
{
public:
	Mesh(const Model& model) : 
		VertexCount(model.Vertices->Count()), IndexCount(model.Indices->size()) {}

	virtual ~Mesh() {}

	const size_t VertexCount;
	const size_t  IndexCount;

	virtual VertexArrayHandle CreateVertexArray(const BufferLayout& instanceLayout) const = 0;

	friend class RenderDevice;
};

using MeshHandle = std::shared_ptr<Mesh>;