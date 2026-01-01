#pragma once

#include <glm/glm.hpp>

#include "Reflection.hpp"
#include "Engine/Rendering/Common.hpp"

class BufferAttribute
{
public:
	BufferAttribute(ElementType type, size_t count, size_t offset) :
		Type(type), Count(count), Offset(offset) {}

	ElementType Type;
	size_t      Count;
	size_t      Offset;

	[[nodiscard]] size_t SizeInBytes() const { return Count * GetTypeSize(); }
private:
	[[nodiscard]] size_t GetTypeSize() const
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
};

class BufferLayout
{
public:
	BufferLayout() : m_stride(0) {}

	void AddAttribute(ElementType type, size_t count)
	{
		const auto& element = m_attributes.emplace_back(type, count, m_stride);
		m_stride += element.SizeInBytes();
	}

	[[nodiscard]] size_t GetStride() const { return m_stride; }

	template<ShallowCopyable T>
	void AddAttribute() {}


	[[nodiscard]] auto begin() const { return m_attributes.begin(); }
	[[nodiscard]] auto end()   const { return m_attributes.end();   }
private:
	std::vector<BufferAttribute> m_attributes;
	size_t m_stride;
};

template<>
inline void BufferLayout::AddAttribute<uint8_t>() { AddAttribute(ElementType::UInt8, 1); }

template<>
inline void BufferLayout::AddAttribute<int8_t>() { AddAttribute(ElementType::SInt8, 1); }

template<>
inline void BufferLayout::AddAttribute<uint32_t>() { AddAttribute(ElementType::UInt32, 1); }

template<>
inline void BufferLayout::AddAttribute<int32_t>() { AddAttribute(ElementType::SInt32, 1); }

template<>
inline void BufferLayout::AddAttribute<glm::ivec2>() { AddAttribute(ElementType::SInt32, 2); }

template<>
inline void BufferLayout::AddAttribute<glm::ivec3>() { AddAttribute(ElementType::SInt32, 3); }

template<>
inline void BufferLayout::AddAttribute<glm::ivec4>() { AddAttribute(ElementType::SInt32, 4); }

template<>
inline void BufferLayout::AddAttribute<float>() { AddAttribute(ElementType::Float32, 1); }

template<>
inline void BufferLayout::AddAttribute<glm::vec2>() { AddAttribute(ElementType::Float32, 2); }

template<>
inline void BufferLayout::AddAttribute<glm::vec3>() { AddAttribute(ElementType::Float32, 3); }

template<>
inline void BufferLayout::AddAttribute<glm::vec4>() { AddAttribute(ElementType::Float32, 4); }

template<>
inline void BufferLayout::AddAttribute<glm::mat2>() { AddAttribute(ElementType::Float32, 2 * 2); }

template<>
inline void BufferLayout::AddAttribute<glm::mat3>() { AddAttribute(ElementType::Float32, 3 * 3); }

template<>
inline void BufferLayout::AddAttribute<glm::mat4>() { AddAttribute(ElementType::Float32, 4 * 4); }

template<>
inline void BufferLayout::AddAttribute<double>() { AddAttribute(ElementType::Float64, 1); }

template<>
inline void BufferLayout::AddAttribute<glm::dvec2>() { AddAttribute(ElementType::Float64, 2); }

template<>
inline void BufferLayout::AddAttribute<glm::dvec3>() { AddAttribute(ElementType::Float64, 3); }

template<>
inline void BufferLayout::AddAttribute<glm::dvec4>() { AddAttribute(ElementType::Float64, 4); }

template<>
inline void BufferLayout::AddAttribute<glm::dmat2>() { AddAttribute(ElementType::Float64, 2 * 2); }

template<>
inline void BufferLayout::AddAttribute<glm::dmat3>() { AddAttribute(ElementType::Float64, 3 * 3); }

template<>
inline void BufferLayout::AddAttribute<glm::dmat4>() { AddAttribute(ElementType::Float64, 4 * 4); }


template<typename T>
concept HasLayout = ShallowCopyable<T> && requires
{
	{ T::GetLayout() } -> std::same_as<BufferLayout>;
};


class DynamicBuffer
{
public:
	explicit DynamicBuffer() = default;

	virtual ~DynamicBuffer() = default;

	virtual TypeInfo* GetElementType() const = 0;

	[[nodiscard]] virtual       void* Data()       = 0;
	[[nodiscard]] virtual const void* Data() const = 0;

	[[nodiscard]] virtual size_t      Count() const = 0;
private:
	const BufferLayout m_layout;
};

template<ShallowCopyable TElement>
class ConstBufferSlice;

template<ShallowCopyable TElement>
class BufferSlice;

template<ShallowCopyable TElement>
class Buffer final : public DynamicBuffer
{
public:
	Buffer(std::initializer_list<TElement> elements) : m_elements { elements } {}

	explicit Buffer(const std::vector<TElement>& elements) :	m_elements(elements) {}

	explicit Buffer(size_t count, const TElement& element)
	{
		m_elements.resize(count, element);
	}

	Buffer(const Buffer& source) : m_elements(source.m_elements) {}

	      TElement& operator[](size_t index)       { return m_elements[index]; }
	const TElement& operator[](size_t index) const { return m_elements[index]; }

	TypeInfo* GetElementType() const override { return TypeInfo::Get<TElement>(); }

	[[nodiscard]]       void* Data()       override { return m_elements.data(); }
	[[nodiscard]] const void* Data() const override { return m_elements.data(); }

	[[nodiscard]] size_t Count() const override { return m_elements.size(); }

	     BufferSlice<TElement> AsSlice();
	ConstBufferSlice<TElement> AsSlice() const;

	     BufferSlice<TElement> AsSlice(size_t index);
	ConstBufferSlice<TElement> AsSlice(size_t index) const;

		 BufferSlice<TElement> AsSlice(size_t index, size_t count);
	ConstBufferSlice<TElement> AsSlice(size_t index, size_t count) const;

	friend class ConstBufferSlice<TElement>;
	friend class BufferSlice<TElement>;
private:
	std::vector<TElement> m_elements;
};

template<ShallowCopyable TElement>
class BufferSlice
{
public:
	BufferSlice(std::nullptr_t) : m_address(nullptr), m_count(0) {}

	BufferSlice(TElement* address, size_t count) :
		m_address(address), m_count(count) {}

	BufferSlice(Buffer<TElement>& array) : BufferSlice(array, 0, array.Count()) {}

	BufferSlice(Buffer<TElement>& array, size_t offset, size_t count) :
		m_address(array.m_elements.data() + offset), m_count(count) {}

	operator bool() const { return m_address; }

	[[nodiscard]] size_t Count() const { return m_count; }

	TElement& operator[](size_t index) const { return m_address[index]; }

	TElement* Data() const { return m_address; }

	BufferSlice Slice(size_t index) const
	{
		return BufferSlice(m_address + index, m_count - index);
	}

	BufferSlice Slice(size_t index, size_t count) const
	{
		return BufferSlice(m_address + index, count);
	}

	void CopyTo(BufferSlice target) const
	{
		memmove(target.m_address, m_address, m_count * sizeof(TElement));
	}
private:
	TElement* m_address;
	size_t    m_count;
};

template<ShallowCopyable TElement>
class ConstBufferSlice
{
public:
	ConstBufferSlice(std::nullptr_t) : m_address(nullptr), m_count(0) {}

	ConstBufferSlice(BufferSlice<TElement> source) :
		m_address(source.Data()), m_count(source.Count()) {}

	ConstBufferSlice(const TElement* address, size_t count) :
		m_address(address), m_count(count) {}

	ConstBufferSlice(const Buffer<TElement>& array) : ConstBufferSlice(array, 0, array.Count()) {}

	ConstBufferSlice(const Buffer<TElement>& array, size_t offset, size_t count) :
		m_address(array.m_elements.data() + offset), m_count(count) {}

	operator bool() const { return m_address; }

	[[nodiscard]] size_t Count() const { return m_count; }

	const TElement& operator[](size_t index) const { return m_address[index]; }

	const TElement* Data() const { return m_address; }

	ConstBufferSlice Slice(size_t index) const
	{
		return ConstBufferSlice(m_address + index, m_count - index);
	}

	ConstBufferSlice Slice(size_t index, size_t count) const
	{
		return ConstBufferSlice(m_address + index, count);
	}

	void CopyTo(BufferSlice<TElement> target) const
	{
		memmove(target.Data(), m_address, m_count * sizeof(TElement));
	}
private:
	const TElement* m_address;
	      size_t    m_count;
};

template<ShallowCopyable TElement>
BufferSlice<TElement> Buffer<TElement>::AsSlice()
{
	return BufferSlice<TElement>(m_elements.data(), m_elements.size());
}

template<ShallowCopyable TElement>
ConstBufferSlice<TElement> Buffer<TElement>::AsSlice() const
{
	return ConstBufferSlice<TElement>(m_elements.data(), m_elements.size());
}

template<ShallowCopyable TElement>
BufferSlice<TElement> Buffer<TElement>::AsSlice(size_t index)
{
	return BufferSlice<TElement>(m_elements.data() + index, m_elements.size() - index);
}

template<ShallowCopyable TElement>
ConstBufferSlice<TElement> Buffer<TElement>::AsSlice(size_t index) const
{
	return ConstBufferSlice<TElement>(m_elements.data() + index, m_elements.size() - index);
}

template<ShallowCopyable TElement>
BufferSlice<TElement> Buffer<TElement>::AsSlice(size_t index, size_t count)
{
	return BufferSlice<TElement>(m_elements.data() + index, count);
}

template<ShallowCopyable TElement>
ConstBufferSlice<TElement> Buffer<TElement>::AsSlice(size_t index, size_t count) const
{
	return ConstBufferSlice<TElement>(m_elements.data() + index, count);
}

class DynamicBufferSlice
{
public:
	DynamicBufferSlice(std::nullptr_t) : m_address(nullptr), m_count(0), m_elementType(nullptr) {}

	DynamicBufferSlice(void* address, size_t count, TypeInfo* elementType) :
		m_address(address), m_count(count), m_elementType(elementType) {}

	DynamicBufferSlice(DynamicBuffer& array) : DynamicBufferSlice(array, 0, array.Count()) {}

	DynamicBufferSlice(DynamicBuffer& array, size_t offset, size_t count) :
		m_address(static_cast<uint8_t*>(array.Data()) + offset * array.GetElementType()->Size),
		m_count(count),
		m_elementType(array.GetElementType()) {}

	template<ShallowCopyable TElement>
	DynamicBufferSlice(BufferSlice<TElement> array) :
		m_address(array.Data()),
		m_count(array.Count()),
		m_elementType(TypeInfo::Get<TElement>()) {}

	operator bool() const { return m_address; }

	[[nodiscard]] size_t Count() const { return m_count; }

	void* Data() const { return m_address; }

	[[nodiscard]] TypeInfo* GetLayout() const { return m_elementType; }

	[[nodiscard]] DynamicBufferSlice Slice(size_t index, size_t count) const
	{
		return DynamicBufferSlice(static_cast<uint8_t*>(m_address) + index * m_elementType->Size, count, m_elementType);
	}
private:
	void* const  m_address;
	const size_t m_count;

	TypeInfo* const m_elementType;
};

class ConstDynamicBufferSlice
{
public:
	ConstDynamicBufferSlice(std::nullptr_t) : m_address(nullptr), m_count(0), m_elementType(nullptr) {}

	ConstDynamicBufferSlice(const void* address, size_t count, TypeInfo* elementType) :
		m_address(address), m_count(count), m_elementType(elementType) {}

	ConstDynamicBufferSlice(const DynamicBuffer& array) : ConstDynamicBufferSlice(array, 0, array.Count()) {}

	ConstDynamicBufferSlice(const DynamicBuffer& array, size_t offset, size_t count) :
		m_address(static_cast<const uint8_t*>(array.Data()) + offset * array.GetElementType()->Size),
		m_count(count),
		m_elementType(array.GetElementType()) {}

	template<ShallowCopyable TElement>
	ConstDynamicBufferSlice(BufferSlice<TElement> array) :
		m_address(array.Data()),
		m_count(array.Count()),
		m_elementType() {}

	template<ShallowCopyable TElement>
	ConstDynamicBufferSlice(ConstBufferSlice<TElement> array) :
		m_address(array.Data()),
		m_count(array.Count()),
		m_elementType(&array.GetLayout()) {}

	operator bool() const { return m_address; }

	[[nodiscard]] size_t Count() const { return m_count; }

	[[nodiscard]] const void* Data() const { return m_address; }

	[[nodiscard]] TypeInfo* GetLayout() const { return m_elementType; }

	[[nodiscard]] ConstDynamicBufferSlice Slice(size_t index, size_t count) const
	{
		return ConstDynamicBufferSlice(static_cast<const uint8_t*>(m_address) + index * m_elementType->Size, count, m_elementType);
	}
private:
	const void* const m_address;
	const size_t      m_count;

	TypeInfo* const m_elementType;
};