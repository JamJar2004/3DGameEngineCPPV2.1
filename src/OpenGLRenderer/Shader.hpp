#pragma once

#include <GL/glew.h>
#include <vector>
#include <string_view>
#include <fstream>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>

namespace GL
{
	enum class UniformType
	{
		Int         = GL_INT,
		Int2        = GL_INT_VEC2,
		Int3        = GL_INT_VEC3,
		Int4        = GL_INT_VEC4,
		Float       = GL_FLOAT,
		Float2      = GL_FLOAT_VEC2,
		Float3      = GL_FLOAT_VEC3,
		Float4      = GL_FLOAT_VEC4,
		Double      = GL_DOUBLE,
		Double2     = GL_DOUBLE_VEC2,
		Double3     = GL_DOUBLE_VEC3,
		Double4     = GL_DOUBLE_VEC4,
		Float2x2    = GL_FLOAT_MAT2,
		Float3x3    = GL_FLOAT_MAT3,
		Float4x4    = GL_FLOAT_MAT4,
		Sampler2D   = GL_SAMPLER_2D,
		Sampler3D   = GL_SAMPLER_3D,
		SamplerCube = GL_SAMPLER_CUBE,
	};

	class Uniform
	{
	public:
		Uniform(size_t index, size_t location, std::string_view name, UniformType type) :
			Index(index), Location(location), Name(name), Type(type) {}

		const size_t           Index;
		const size_t           Location;
		const std::string_view Name;
		const UniformType      Type;
	};

	class UniformCollection;

	class Shader
	{
	public:
		Shader(uint32_t version, std::string_view sourceCode);

		~Shader();

		void Bind() { glUseProgram(m_programID); }

		void SetUniform(std::string_view name, int value);
		void SetUniform(std::string_view name, const glm::ivec2& value);
		void SetUniform(std::string_view name, const glm::ivec3& value);
		void SetUniform(std::string_view name, const glm::ivec4& value);
		void SetUniform(std::string_view name, float value);
		void SetUniform(std::string_view name, const glm::vec2& value);
		void SetUniform(std::string_view name, const glm::vec3& value);
		void SetUniform(std::string_view name, const glm::vec4& value);
		void SetUniform(std::string_view name, const glm::mat2& value);
		void SetUniform(std::string_view name, const glm::mat3& value);
		void SetUniform(std::string_view name, const glm::mat4& value);

		UniformCollection GetUniforms();

		friend class UniformIterator;

	private:
		GLuint m_programID;

		std::vector<GLuint> m_shaders;

		std::unordered_map<std::string, GLint> m_uniformLocations;

		GLint StoreUniformLocation(std::string_view name) const;

		GLint RegisterUniform(std::string_view name);

		void Compile();

		void AddShader(std::string_view text, GLenum type);
	};

	class UniformIterator
	{

	public:
		UniformIterator(Shader& shader, bool end);

		UniformIterator& operator++();

		Uniform operator*() const;

		bool operator==(const UniformIterator& other) const;
	private:
		Shader& m_shader;
		size_t  m_index;
	};

	class UniformCollection
	{
	public:
		explicit UniformCollection(Shader& shader) : m_shader(shader) {}

		UniformIterator begin() const { return UniformIterator(m_shader, false); }
		UniformIterator end()   const { return UniformIterator(m_shader, true);  }
	private:
		Shader& m_shader;
	};
}