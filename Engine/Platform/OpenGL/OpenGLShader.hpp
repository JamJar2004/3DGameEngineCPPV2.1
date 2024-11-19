#pragma once

#include "../../Rendering/Shader.hpp"

#include <OpenGLRenderer/Shader.hpp>

class OpenGLShader : public Shader
{
private:
	GL::Shader m_shader;

	std::unordered_map<std::string, std::string> m_materialNameMap;

	template<typename T>
	void SetUniform(const std::string& name, const void* address);

	void SetUniform(Type* type, const std::string& name, const void* source);
public:
	OpenGLShader(const std::string& vertexShaderText, const std::string& fragmentShaderText);

	virtual void OnUse(const void* materialData) override;
};

template<typename T>
inline void OpenGLShader::SetUniform(const std::string& name, const void* address)
{
	const T& value = *(T*)address;
	m_shader.SetUniform(name, value);
}
