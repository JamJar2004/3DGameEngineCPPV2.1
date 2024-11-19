#include "OpenGLShader.hpp"

Type* GetType(GL::UniformType uniformType)
{
	switch(uniformType)
	{
		case GL::UniformType::Int:
			return Type::Get<int>();
		case GL::UniformType::Int2:
			return Type::Get<glm::ivec2>();
		case GL::UniformType::Int3:
			return Type::Get<glm::ivec3>();
		case GL::UniformType::Int4:
			return Type::Get<glm::ivec4>();
		case GL::UniformType::Float:
			return Type::Get<float>();
		case GL::UniformType::Float2:
			return Type::Get<glm::vec2>();
		case GL::UniformType::Float3:
			return Type::Get<glm::vec3>();
		case GL::UniformType::Float4:
			return Type::Get<glm::vec4>();
		case GL::UniformType::Float2x2:
			return Type::Get<glm::mat2>();
		case GL::UniformType::Float3x3:
			return Type::Get<glm::mat3>();
		case GL::UniformType::Float4x4:
			return Type::Get<glm::mat4>();
		case GL::UniformType::Sampler2D:
		case GL::UniformType::Sampler3D:
		case GL::UniformType::SamplerCube:
			return Type::Get<int>();
	}

	return nullptr;
}

void OpenGLShader::SetUniform(Type* type, const std::string& name, const void* source)
{
	if(type == Type::Get<int>())
		SetUniform<int>(name, source);
	else if(type == Type::Get<glm::ivec2>())
		SetUniform<glm::ivec2>(name, source);
	else if(type == Type::Get<glm::ivec3>())
		SetUniform<glm::ivec3>(name, source);
	else if(type == Type::Get<glm::ivec4>())
		SetUniform<glm::ivec4>(name, source);
	else if(type == Type::Get<float>())
		SetUniform<float>(name, source);
	else if(type == Type::Get<glm::vec2>())
		SetUniform<glm::vec2>(name, source);
	else if(type == Type::Get<glm::vec3>())
		SetUniform<glm::vec3>(name, source);
	else if(type == Type::Get<glm::vec4>())
		SetUniform<glm::vec4>(name, source);
	else if(type == Type::Get<glm::mat2>())
		SetUniform<glm::mat2>(name, source);
	else if(type == Type::Get<glm::mat3>())
		SetUniform<glm::mat3>(name, source);
	else if(type == Type::Get<glm::mat4>())
		SetUniform<glm::mat4>(name, source);
}

OpenGLShader::OpenGLShader(const std::string& vertexShaderText, const std::string& fragmentShaderText) :
	m_shader(vertexShaderText, fragmentShaderText)
{
	for(GL::Uniform uniform : m_shader.GetUniforms())
	{
		bool isMaterialUniform = uniform.Name.starts_with("m_");

		std::string uniformName = uniform.Name;
		if(isMaterialUniform)
			uniformName = uniformName.substr(2);

		Field* field;
		if(isMaterialUniform)
		{
			m_materialNameMap[uniformName] = uniform.Name;
			field = &AddMaterialField(uniformName, GetType(uniform.Type));
		}
		else
			field = &AddUniformField(uniform.Name, GetType(uniform.Type));

		if(uniform.Type == GL::UniformType::Sampler2D || uniform.Type == GL::UniformType::Sampler3D || uniform.Type == GL::UniformType::SamplerCube)
		{
			if(isMaterialUniform)
				AddMaterialTexture(uniformName);
			else
				AddUniformTexture(uniform.Name);
		}
	}
}

void OpenGLShader::OnUse(const void* materialData)
{
	m_shader.Bind();
	for(const Field& field : GetMaterialFields())
	{
		const void* source = ((const uint8_t*)materialData) + field.Offset;
		SetUniform(field.Type, m_materialNameMap[field.Name], source);
	}

	for(const Field& field : GetUniformFields())
	{
		const void* source = ((const uint8_t*)UniformData()) + field.Offset;
		SetUniform(field.Type, field.Name, source);
	}
}
