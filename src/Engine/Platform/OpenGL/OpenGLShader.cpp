#include "OpenGLShader.hpp"

TypeInfo* GetType(GL::UniformType uniformType)
{
	switch(uniformType)
	{
		case GL::UniformType::Int:
			return TypeInfo::Get<glm::ivec1>();
		case GL::UniformType::Int2:
			return TypeInfo::Get<glm::ivec2>();
		case GL::UniformType::Int3:
			return TypeInfo::Get<glm::ivec3>();
		case GL::UniformType::Int4:
			return TypeInfo::Get<glm::ivec4>();
		case GL::UniformType::Float:
			return TypeInfo::Get<glm::fvec1>();
		case GL::UniformType::Float2:
			return TypeInfo::Get<glm::fvec2>();
		case GL::UniformType::Float3:
			return TypeInfo::Get<glm::fvec3>();
		case GL::UniformType::Float4:
			return TypeInfo::Get<glm::fvec4>();
		case GL::UniformType::Float2x2:
			return TypeInfo::Get<glm::fmat2>();
		case GL::UniformType::Float3x3:
			return TypeInfo::Get<glm::fmat3>();
		case GL::UniformType::Float4x4:
			return TypeInfo::Get<glm::fmat4>();
		case GL::UniformType::Sampler2D:
		case GL::UniformType::Sampler3D:
		case GL::UniformType::SamplerCube:
			return TypeInfo::Get<int>();
	}

	return nullptr;
}

void OpenGLShader::GetUniforms(std::vector<Uniform>& uniforms)
{
	for (GL::Uniform uniform : m_shader.GetUniforms())
	{
		TypeInfo* uniformType;
		switch(uniform.Type)
		{
			case GL::UniformType::Int:
				uniformType = TypeInfo::Get<int32_t>();
				break;
			case GL::UniformType::Int2:
				uniformType = TypeInfo::Get<glm::ivec2>();
				break;
			case GL::UniformType::Int3:
				uniformType = TypeInfo::Get<glm::ivec3>();
				break;
			case GL::UniformType::Int4:
				uniformType = TypeInfo::Get<glm::ivec4>();
				break;
			case GL::UniformType::Float:
				uniformType = TypeInfo::Get<float>();
				break;
			case GL::UniformType::Float2:
				uniformType = TypeInfo::Get<glm::fvec2>();
				break;
			case GL::UniformType::Float3:
				uniformType = TypeInfo::Get<glm::fvec3>();
				break;
			case GL::UniformType::Float4:
				uniformType = TypeInfo::Get<glm::fvec4>();
				break;
			case GL::UniformType::Double:
				uniformType = TypeInfo::Get<double>();
				break;
			case GL::UniformType::Double2:
				uniformType = TypeInfo::Get<glm::dvec2>();
				break;
			case GL::UniformType::Double3:
				uniformType = TypeInfo::Get<glm::dvec3>();
				break;
			case GL::UniformType::Double4:
				uniformType = TypeInfo::Get<glm::dvec4>();
				break;
			case GL::UniformType::Float2x2:
				uniformType = TypeInfo::Get<glm::fmat2>();
				break;
			case GL::UniformType::Float3x3:
				uniformType = TypeInfo::Get<glm::fmat3>();
				break;
			case GL::UniformType::Float4x4:
				uniformType = TypeInfo::Get<glm::fmat4>();
				break;
			default: continue;
		}

		uniforms.emplace_back(std::string(uniform.Name), uniformType);
	}
}

void OpenGLShader::SetUniform(TypeInfo* type, std::string_view name, const void* source)
{
	if(type == TypeInfo::Get<int>())
		SetUniform<int32_t>(name, source);
	else if(type == TypeInfo::Get<glm::ivec2>())
		SetUniform<glm::ivec2>(name, source);
	else if(type == TypeInfo::Get<glm::ivec3>())
		SetUniform<glm::ivec3>(name, source);
	else if(type == TypeInfo::Get<glm::ivec4>())
		SetUniform<glm::ivec4>(name, source);
	else if(type == TypeInfo::Get<float>())
		SetUniform<float>(name, source);
	else if(type == TypeInfo::Get<glm::vec2>())
		SetUniform<glm::vec2>(name, source);
	else if(type == TypeInfo::Get<glm::vec3>())
		SetUniform<glm::vec3>(name, source);
	else if(type == TypeInfo::Get<glm::vec4>())
		SetUniform<glm::vec4>(name, source);
	else if(type == TypeInfo::Get<glm::mat2>())
		SetUniform<glm::mat2>(name, source);
	else if(type == TypeInfo::Get<glm::mat3>())
		SetUniform<glm::mat3>(name, source);
	else if(type == TypeInfo::Get<glm::mat4>())
		SetUniform<glm::mat4>(name, source);
}

OpenGLShader::OpenGLShader(std::string_view sourceCode) :
	Shader(sourceCode),
	m_shader(430, sourceCode) {}

void OpenGLShader::Use()
{
	m_shader.Bind();
	// for(const Field& field : GetMaterialFields())
	// {
	// 	const void* source = static_cast<const uint8_t*>(materialData) + field.Offset;
	// 	SetUniform(field.Type, m_materialNameMap[field.Name], source);
	// }
	//
	// for(const Field& field : GetUniformFields())
	// {
	// 	const void* source = static_cast<const uint8_t*>(UniformData()) + field.Offset;
	// 	SetUniform(field.Type, field.Name, source);
	// }
}
