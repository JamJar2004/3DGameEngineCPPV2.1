#pragma once

#include <GL/glew.h>
#include <vector>
#include <string_view>
#include <fstream>
#include <string>
#include <iostream>
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
		Uniform(size_t index, size_t location, const std::string& name, UniformType type) :
			Index(index), Location(location), Name(name), Type(type) {}

		const size_t      Index;
		const size_t      Location;
		const std::string Name;
		const UniformType Type;
	};

	class UniformCollection;

	class Shader
	{
	private:
		GLuint m_programID;

		std::vector<GLuint> m_shaders;

		std::unordered_map<std::string, GLint> m_uniformLocations;

		GLint AddUniform(const std::string& name);

		GLint RegisterUniform(const std::string& name);

		void CheckError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage);

		void Compile();

		void AddShader(const std::string& text, GLenum type);
	public:
		Shader(const std::string& vertexShaderText, const std::string& fragmentShaderText);

		~Shader();

		void Bind() { glUseProgram(m_programID); }

		void SetUniform(const std::string& name, int value);
		void SetUniform(const std::string& name, const glm::ivec2& value);
		void SetUniform(const std::string& name, const glm::ivec3& value);
		void SetUniform(const std::string& name, const glm::ivec4& value);
		void SetUniform(const std::string& name, float value);
		void SetUniform(const std::string& name, const glm::vec2& value);
		void SetUniform(const std::string& name, const glm::vec3& value);
		void SetUniform(const std::string& name, const glm::vec4& value);
		void SetUniform(const std::string& name, const glm::mat2& value);
		void SetUniform(const std::string& name, const glm::mat3& value);
		void SetUniform(const std::string& name, const glm::mat4& value);

		UniformCollection GetUniforms();

		friend class UniformIterator;
	};

	class UniformIterator
	{
	private:
		Shader& m_shader;
		size_t  m_index;
	public:
		UniformIterator(Shader& shader, bool end);

		UniformIterator& operator++();

		Uniform operator*() const;

		bool operator==(const UniformIterator& other) const;
	};

	class UniformCollection
	{
	private:
		Shader& m_shader;
	public:
		UniformCollection(Shader& shader) : m_shader(shader) {}

		UniformIterator begin() const { return UniformIterator(m_shader, false); }
		UniformIterator end()   const { return UniformIterator(m_shader, true);  }
	};
}