#include "Shader.hpp"

#include <iostream>
#include <sstream>

namespace GL
{
	GLint Shader::StoreUniformLocation(std::string_view name) const
	{
		return glGetUniformLocation(m_programID, name.data());
	}

	GLint Shader::RegisterUniform(std::string_view name)
	{
		std::string nameKey(name);
		auto it = m_uniformLocations.find(nameKey);
		if(it == m_uniformLocations.end())
		{
			GLint result = StoreUniformLocation(name);
			m_uniformLocations[nameKey] = result;
			return result;
		}

		return m_uniformLocations[nameKey];
	}

	static void CheckError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage)
	{
		GLint success = 0;
		GLchar error[1024] = { 0 };

		if(isProgram)
			glGetProgramiv(shader, flag, &success);
		else
			glGetShaderiv(shader, flag, &success);

		if(success == GL_FALSE)
		{
			if(isProgram)
			{
				glGetProgramInfoLog(shader, sizeof(error), nullptr, error);
			}
			else
			{
				glGetShaderInfoLog(shader, sizeof(error), nullptr, error);
			}

			std::cerr << errorMessage << ": '" << error << "'" << std::endl;
		}
	}

	void Shader::Compile()
	{
		glLinkProgram(m_programID);
		CheckError(m_programID, GL_LINK_STATUS, true, "Error linking shader program");

		glValidateProgram(m_programID);
		CheckError(m_programID, GL_LINK_STATUS, true, "Invalid shader program");
	}

	void Shader::AddShader(std::string_view text, GLenum type)
	{
		const int shader = glCreateShader(type);

		if(shader == 0)
		{
			std::cerr << "Shader creation failed: Could not find valid memory location when adding shader" << std::endl;
		}

		const GLchar* textChars[] { text.data() };
		const GLint textLengths[] { static_cast<GLint>(text.length()) };

		glShaderSource(shader, 1, textChars, textLengths);
		glCompileShader(shader);

		CheckError(shader, GL_COMPILE_STATUS, false, "Failed to compile shader!");

		glAttachShader(m_programID, shader);
		m_shaders.push_back(shader);
	}

	Shader::Shader(uint32_t version, std::string_view sourceCode)
	{
		m_programID = glCreateProgram();

		std::stringstream vertexStream;
		vertexStream << "#version " << version << "\n";
		vertexStream << "#define VERTEX_SHADER\n";
		vertexStream << sourceCode;

		std::stringstream geometryStream;
		geometryStream << "#version " << version << "\n";
		geometryStream << "#define GEOMETRY_SHADER\n";
		geometryStream << sourceCode;

		std::stringstream fragmentStream;
		fragmentStream << "#version " << version << "\n";
		fragmentStream << "#define FRAGMENT_SHADER\n";
		fragmentStream << sourceCode;

		AddShader(vertexStream.str(), GL_VERTEX_SHADER);
		AddShader(vertexStream.str(), GL_GEOMETRY_SHADER);
		AddShader(fragmentStream.str(), GL_FRAGMENT_SHADER);

		Compile();
	}

	Shader::~Shader()
	{
		for(const GLuint shader : m_shaders)
		{
			glDeleteShader(shader);
		}
		glDeleteProgram(m_programID);
	}

	void Shader::SetUniform(std::string_view name, int value)
	{
		const GLint location = RegisterUniform(name);
		glUniform1i(location, value);
	}

	void Shader::SetUniform(std::string_view name, const glm::ivec2& value)
	{
		const GLint location = RegisterUniform(name);
		glUniform2i(location, value.x, value.y);
	}

	void Shader::SetUniform(std::string_view name, const glm::ivec3& value)
	{
		const GLint location = RegisterUniform(name);
		glUniform3i(location, value.x, value.y, value.z);
	}

	void Shader::SetUniform(std::string_view name, const glm::ivec4& value)
	{
		const GLint location = RegisterUniform(name);
		glUniform4i(location, value.x, value.y, value.z, value.w);
	}

	void Shader::SetUniform(std::string_view name, float value)
	{
		const GLint location = RegisterUniform(name);
		glUniform1f(location, value);
	}

	void Shader::SetUniform(std::string_view name, const glm::vec2& value)
	{
		const GLint location = RegisterUniform(name);
		glUniform2f(location, value.x, value.y);
	}

	void Shader::SetUniform(std::string_view name, const glm::vec3& value)
	{
		const GLint location = RegisterUniform(name);
		glUniform3f(location, value.x, value.y, value.z);
	}

	void Shader::SetUniform(std::string_view name, const glm::vec4& value)
	{
		const GLint location = RegisterUniform(name);
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void Shader::SetUniform(std::string_view name, const glm::mat2& value)
	{
		const GLint location = RegisterUniform(name);
		glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::SetUniform(std::string_view name, const glm::mat3& value)
	{
		const GLint location = RegisterUniform(name);
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::SetUniform(std::string_view name, const glm::mat4& value)
	{
		const GLint location = RegisterUniform(name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	UniformCollection Shader::GetUniforms()
	{
		return UniformCollection(*this);
	}

	UniformIterator::UniformIterator(Shader& shader, bool end) : m_shader(shader), m_index(0) 
	{
		if(end)
		{ 
			GLint endIndex;
			glGetProgramiv(shader.m_programID, GL_ACTIVE_UNIFORMS, &endIndex);
			m_index = endIndex;
		}
	}

	UniformIterator& UniformIterator::operator++()
	{
		++m_index;
		return *this;
	}

	Uniform UniformIterator::operator*() const
	{
		char name[1024];
		GLenum type;

		glGetActiveUniform(m_shader.m_programID, static_cast<GLuint>(m_index), 1024, nullptr, nullptr, &type, name);

		return Uniform(m_index, m_shader.RegisterUniform(name), name, static_cast<UniformType>(type));
	}

	bool UniformIterator::operator==(const UniformIterator& other) const
	{
		return m_index == other.m_index;
	}
}
