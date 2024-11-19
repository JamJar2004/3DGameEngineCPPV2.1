#include "Shader.hpp"

namespace GL
{
	GLint Shader::AddUniform(const std::string& name)
	{
		return glGetUniformLocation(m_programID, name.c_str());
	}

	GLint Shader::RegisterUniform(const std::string& name)
	{
		auto it = m_uniformLocations.find(name);
		if(it == m_uniformLocations.end())
		{
			GLint result = AddUniform(name);
			m_uniformLocations[name] = result;
			return result;
		}

		return m_uniformLocations[name];
	}

	void Shader::CheckError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage)
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
				glGetProgramInfoLog(shader, sizeof(error), nullptr, error);
			else
				glGetShaderInfoLog(shader, sizeof(error), nullptr, error);

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

	void Shader::AddShader(const std::string& text, GLenum type)
	{
		int shader = glCreateShader(type);

		if(shader == 0)
			std::cout << "Shader creation failed: Could not find valid memory location when adding shader" << std::endl;

		const GLchar* textChars[] { text.c_str() };
		GLint textLengths[] { GLint(text.length()) };

		glShaderSource(shader, 1, textChars, textLengths);
		glCompileShader(shader);

		CheckError(shader, GL_COMPILE_STATUS, false, "Failed to compile shader!");

		glAttachShader(m_programID, shader);
		m_shaders.push_back(shader);
	}

	Shader::Shader(const std::string& vertexShaderText, const std::string& fragmentShaderText) : m_programID(0)
	{
		m_programID = glCreateProgram();

		AddShader(vertexShaderText, GL_VERTEX_SHADER);
		AddShader(fragmentShaderText, GL_FRAGMENT_SHADER);

		Compile();
	}

	Shader::~Shader()
	{
		for(GLuint shader : m_shaders)
			glDeleteShader(shader);

		glDeleteProgram(m_programID);
	}

	void Shader::SetUniform(const std::string& name, int value)
	{
		GLint location = RegisterUniform(name);
		glUniform1i(location, value);
	}

	void Shader::SetUniform(const std::string& name, const glm::ivec2& value)
	{
		GLint location = RegisterUniform(name);
		glUniform2i(location, value.x, value.y);
	}

	void Shader::SetUniform(const std::string& name, const glm::ivec3& value)
	{
		GLint location = RegisterUniform(name);
		glUniform3i(location, value.x, value.y, value.z);
	}

	void Shader::SetUniform(const std::string& name, const glm::ivec4& value)
	{
		GLint location = RegisterUniform(name);
		glUniform4i(location, value.x, value.y, value.z, value.w);
	}

	void Shader::SetUniform(const std::string& name, float value)
	{
		GLint location = RegisterUniform(name);
		glUniform1f(location, value);
	}

	void Shader::SetUniform(const std::string& name, const glm::vec2& value)
	{
		GLint location = RegisterUniform(name);
		glUniform2f(location, value.x, value.y);
	}

	void Shader::SetUniform(const std::string& name, const glm::vec3& value)
	{
		GLint location = RegisterUniform(name);
		glUniform3f(location, value.x, value.y, value.z);
	}

	void Shader::SetUniform(const std::string& name, const glm::vec4& value)
	{
		GLint location = RegisterUniform(name);
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void Shader::SetUniform(const std::string& name, const glm::mat2& value)
	{
		GLint location = RegisterUniform(name);
		glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::SetUniform(const std::string& name, const glm::mat3& value)
	{
		GLint location = RegisterUniform(name);
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::SetUniform(const std::string& name, const glm::mat4& value)
	{
		GLint location = RegisterUniform(name);
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

		glGetActiveUniform(m_shader.m_programID, GLuint(m_index), 1024, nullptr, nullptr, &type, name);

		return Uniform(m_index, m_shader.RegisterUniform(name), name, (UniformType)type);
	}

	bool UniformIterator::operator==(const UniformIterator& other) const
	{
		return m_index == other.m_index;
	}
}