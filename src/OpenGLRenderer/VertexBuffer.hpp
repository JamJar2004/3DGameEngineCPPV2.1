#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Common.hpp"

namespace GL
{ 
	class VertexArray;

	class BufferElement
	{
	public:
		BufferElement(ElementType type, size_t count, size_t offset) : Type(type), Count(count), Offset(offset) {}

		ElementType Type;
		size_t      Count;
		size_t      Offset;

		size_t SizeInBytes() const { return Count * GetTypeSize(Type); }
	};

	class BufferLayout
	{
	private:
		std::vector<BufferElement> m_elements;
		size_t m_stride;
	public:
		BufferLayout() : m_stride(0) {}

		void AddElement(ElementType type, size_t count)
		{
			const BufferElement& element = m_elements.emplace_back(type, count, m_stride);
			m_stride += element.SizeInBytes();
		}

		size_t GetStride() const { return m_stride; }

		template<typename T>
		void AddElement() {}


		auto begin() const { return m_elements.begin(); }
		auto end()   const { return m_elements.end();   }
	};

	template<>
	inline void BufferLayout::AddElement<uint8_t>() { AddElement(ElementType::UInt8, 1); }

	template<>
	inline void BufferLayout::AddElement<int8_t>() { AddElement(ElementType::SInt8, 1); }

	template<>
	inline void BufferLayout::AddElement<uint32_t>() { AddElement(ElementType::UInt32, 1); }

	template<>
	inline void BufferLayout::AddElement<int32_t>() { AddElement(ElementType::SInt32, 1); }

	template<>
	inline void BufferLayout::AddElement<glm::ivec2>() { AddElement(ElementType::SInt32, 2); }

	template<>
	inline void BufferLayout::AddElement<glm::ivec3>() { AddElement(ElementType::SInt32, 3); }

	template<>
	inline void BufferLayout::AddElement<glm::ivec4>() { AddElement(ElementType::SInt32, 4); }

	template<>
	inline void BufferLayout::AddElement<float>() { AddElement(ElementType::Float32, 1); }

	template<>
	inline void BufferLayout::AddElement<glm::vec2>() { AddElement(ElementType::Float32, 2); }

	template<>
	inline void BufferLayout::AddElement<glm::vec3>() { AddElement(ElementType::Float32, 3); }

	template<>
	inline void BufferLayout::AddElement<glm::vec4>() { AddElement(ElementType::Float32, 4); }

	template<>
	inline void BufferLayout::AddElement<glm::mat2>() { AddElement(ElementType::Float32, sizeof(glm::mat2) / sizeof(float)); }

	template<>
	inline void BufferLayout::AddElement<glm::mat3>() { AddElement(ElementType::Float32, sizeof(glm::mat3) / sizeof(float)); }

	template<>
	inline void BufferLayout::AddElement<glm::mat4>() { AddElement(ElementType::Float32, sizeof(glm::mat4) / sizeof(float)); }

	template<>
	inline void BufferLayout::AddElement<double>() { AddElement(ElementType::Float64, 1); }

	template<>
	inline void BufferLayout::AddElement<glm::dvec2>() { AddElement(ElementType::Float64, 2); }

	template<>
	inline void BufferLayout::AddElement<glm::dvec3>() { AddElement(ElementType::Float64, 3); }

	template<>
	inline void BufferLayout::AddElement<glm::dvec4>() { AddElement(ElementType::Float64, 4); }

	template<>
	inline void BufferLayout::AddElement<glm::dmat2>() { AddElement(ElementType::Float64, sizeof(glm::dmat2) / sizeof(double)); }

	template<>
	inline void BufferLayout::AddElement<glm::dmat3>() { AddElement(ElementType::Float64, sizeof(glm::dmat3) / sizeof(double)); }

	template<>
	inline void BufferLayout::AddElement<glm::dmat4>() { AddElement(ElementType::Float64, sizeof(glm::dmat4) / sizeof(double)); }



	class VertexBuffer
	{
	private:
		const BufferLayout m_layout;

		GLuint m_bufferID;

		void Bind();
	public:
		VertexBuffer(size_t count, BufferUsage usage, bool isInstanced, const BufferLayout& layout);
		VertexBuffer(const void* data, size_t count, BufferUsage usage, bool isInstanced, const BufferLayout& layout);

		~VertexBuffer();

		const size_t VertexCount;
		const bool   IsInstanced;

		const BufferLayout& GetLayout() const { return m_layout; }

		void SetData(const void* data, size_t offset, size_t size);

		friend class GL::VertexArray;
	};
}