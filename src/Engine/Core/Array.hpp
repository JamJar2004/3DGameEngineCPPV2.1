#pragma once

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

	template<typename T>
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
concept HasLayout = requires
{
	{ T::GetLayout() } -> std::same_as<BufferLayout>;
};

class DynamicArray
{
public:
	explicit DynamicArray(BufferLayout layout) : m_layout(std::move(layout)) {}

	virtual ~DynamicArray() = default;

	[[nodiscard]] const BufferLayout& GetLayout() const { return m_layout; }

	[[nodiscard]] virtual const void* Data() const = 0;
	[[nodiscard]] virtual size_t      Count() const = 0;
private:
	const BufferLayout m_layout;
};

template<HasLayout T>
class ConstArraySlice;

template<HasLayout T>
class ArraySlice;

template<HasLayout TElement>
class Array final : public DynamicArray
{
public:
	Array(std::initializer_list<TElement> elements) :
			DynamicArray(TElement::GetLayout()),
			m_elements { elements } {}

	explicit Array(const std::vector<TElement>& elements) :
		DynamicArray(TElement::GetLayout()),
		m_elements(elements) {}

	explicit Array(size_t count, const TElement& element) :
		DynamicArray(TElement::GetLayout())
	{
		m_elements.resize(count, element);
	}

	Array(const Array& source) :
		DynamicArray(TElement::GetLayout()),
		m_elements(source.m_elements) {}

	      TElement& operator[](size_t index)       { return m_elements[index]; }
	const TElement& operator[](size_t index) const { return m_elements[index]; }

	[[nodiscard]] const void* Data() const override { return m_elements.data(); }

	[[nodiscard]] size_t Count() const override { return m_elements.size(); }

	     ArraySlice<TElement> AsSlice();
	ConstArraySlice<TElement> AsSlice() const;

	     ArraySlice<TElement> AsSlice(size_t index);
	ConstArraySlice<TElement> AsSlice(size_t index) const;

		 ArraySlice<TElement> AsSlice(size_t index, size_t count);
	ConstArraySlice<TElement> AsSlice(size_t index, size_t count) const;

	friend class ConstArraySlice<TElement>;
	friend class ArraySlice<TElement>;
private:
	std::vector<TElement> m_elements;
};

template<HasLayout TElement>
class ArraySlice
{
public:
	ArraySlice(std::nullptr_t) : m_address(nullptr), m_count(0), m_layout(nullptr) {}

	ArraySlice(TElement* address, size_t count, const BufferLayout& layout) :
		m_address(address), m_count(count), m_layout(&layout) {}

	ArraySlice(const Array<TElement>& array) : ArraySlice(array, 0, array.Count()) {}

	ArraySlice(const Array<TElement>& array, size_t offset, size_t count) :
		m_address(array.m_elements.data() + offset), m_count(count), m_layout(&array.GetLayout()) {}

	operator bool() const { return m_address; }

	[[nodiscard]] size_t Count() const { return m_count; }

	TElement& operator[](size_t index) const { return m_address[index]; }

	TElement* Data() const { return m_address; }

	[[nodiscard]] const BufferLayout& GetLayout() const { return *m_layout; }

	ArraySlice Slice(size_t index, size_t count) const
	{
		return ArraySlice(m_address + index, count, *m_layout);
	}

	void CopyTo(ArraySlice target) const
	{
		memmove(target.m_address, m_address, m_count * sizeof(TElement));
	}
private:
	TElement* const m_address;
	const size_t    m_count;

	const BufferLayout* m_layout;
};

template<HasLayout TElement>
class ConstArraySlice
{
public:
	ConstArraySlice(std::nullptr_t) : m_address(nullptr), m_count(0), m_layout(nullptr) {}

	ConstArraySlice(ArraySlice<TElement> source) :
		m_address(source.Data()), m_count(source.Count()), m_layout(source.GetLayout()) {}

	ConstArraySlice(const TElement* address, size_t count, const BufferLayout& layout) :
		m_address(address), m_count(count), m_layout(&layout) {}

	ConstArraySlice(const Array<TElement>& array) : ConstArraySlice(array, 0, array.Count()) {}

	ConstArraySlice(const Array<TElement>& array, size_t offset, size_t count) :
		m_address(array.m_elements.data() + offset), m_count(count), m_layout(&array.GetLayout()) {}

	operator bool() const { return m_address; }

	[[nodiscard]] size_t Count() const { return m_count; }

	const TElement& operator[](size_t index) const { return m_address[index]; }

	const TElement* Data() const { return m_address; }

	[[nodiscard]] const BufferLayout& GetLayout() const { return *m_layout; }

	ConstArraySlice Slice(size_t index, size_t count) const
	{
		return ConstArraySlice(m_address + index, count, *m_layout);
	}

	void CopyTo(ArraySlice<TElement> target) const
	{
		memmove(target.Data(), m_address, m_count * sizeof(TElement));
	}
private:
	const TElement* const m_address;
	const size_t          m_count;

	const BufferLayout* m_layout;
};

template<HasLayout TElement>
ArraySlice<TElement> Array<TElement>::AsSlice()
{
	return ArraySlice<TElement>(m_elements);
}

template<HasLayout TElement>
ConstArraySlice<TElement> Array<TElement>::AsSlice() const
{
	return ConstArraySlice<TElement>(m_elements);
}

template<HasLayout TElement>
ArraySlice<TElement> Array<TElement>::AsSlice(size_t index)
{
	return ArraySlice<TElement>(m_elements, index, m_elements.size() - index);
}

template<HasLayout TElement>
ConstArraySlice<TElement> Array<TElement>::AsSlice(size_t index) const
{
	return ConstArraySlice<TElement>(m_elements, index, m_elements.size() - index);
}

template<HasLayout TElement>
ArraySlice<TElement> Array<TElement>::AsSlice(size_t index, size_t count)
{
	return ArraySlice<TElement>(m_elements, index, count);
}

template<HasLayout TElement>
ConstArraySlice<TElement> Array<TElement>::AsSlice(size_t index, size_t count) const
{
	return ConstArraySlice<TElement>(m_elements, index, count);
}

class DynamicArraySlice
{
public:
	DynamicArraySlice(std::nullptr_t) : m_address(nullptr), m_count(0), m_layout(nullptr) {}

	DynamicArraySlice(void* address, size_t count, const BufferLayout& layout) :
		m_address(address), m_count(count), m_layout(&layout) {}

	DynamicArraySlice(DynamicArray& array) : DynamicArraySlice(array, 0, array.Count()) {}

	DynamicArraySlice(DynamicArray& array, size_t offset, size_t count) :
		m_address((uint8_t*)(array.Data()) + offset * array.GetLayout().GetStride()),
		m_count(count),
		m_layout(&array.GetLayout()) {}

	template<HasLayout T>
	DynamicArraySlice(ArraySlice<T> array) :
		m_address(array.Data()),
		m_count(array.Count()),
		m_layout(&array.GetLayout()) {}

	operator bool() const { return m_address; }

	[[nodiscard]] size_t Count() const { return m_count; }

	void* Data() const { return m_address; }

	[[nodiscard]] const BufferLayout& GetLayout() const { return *m_layout; }

	[[nodiscard]] DynamicArraySlice Slice(size_t index, size_t count) const
	{
		return DynamicArraySlice(static_cast<uint8_t*>(m_address) + index * m_layout->GetStride(), count, *m_layout);
	}
private:
	void* const  m_address;
	const size_t m_count;

	const BufferLayout* m_layout;
};

class ConstDynamicArraySlice
{
public:
	ConstDynamicArraySlice(std::nullptr_t) : m_address(nullptr), m_count(0), m_layout(nullptr) {}

	ConstDynamicArraySlice(const void* address, size_t count, const BufferLayout& layout) :
		m_address(address), m_count(count), m_layout(&layout) {}

	ConstDynamicArraySlice(const DynamicArray& array) : ConstDynamicArraySlice(array, 0, array.Count()) {}

	ConstDynamicArraySlice(const DynamicArray& array, size_t offset, size_t count) :
		m_address((uint8_t*)(array.Data()) + offset * array.GetLayout().GetStride()),
		m_count(count),
		m_layout(&array.GetLayout()) {}

	template<HasLayout T>
	ConstDynamicArraySlice(ArraySlice<T> array) :
		m_address(array.Data()),
		m_count(array.Count()),
		m_layout(&array.GetLayout()) {}

	template<HasLayout T>
	ConstDynamicArraySlice(ConstArraySlice<T> array) :
		m_address(array.Data()),
		m_count(array.Count()),
		m_layout(&array.GetLayout()) {}

	operator bool() const { return m_address; }

	[[nodiscard]] size_t Count() const { return m_count; }

	[[nodiscard]] const void* Data() const { return m_address; }

	[[nodiscard]] const BufferLayout& GetLayout() const { return *m_layout; }

	[[nodiscard]] ConstDynamicArraySlice Slice(size_t index, size_t count) const { return ConstDynamicArraySlice((uint8_t*)m_address + index * m_layout->GetStride(), count, *m_layout); }
private:
	const void* const m_address;
	const size_t      m_count;

	const BufferLayout* m_layout;
};