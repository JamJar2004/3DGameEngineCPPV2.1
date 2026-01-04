#pragma once

#include "../../Rendering/Shader.hpp"

#include <OpenGLRenderer/Shader.hpp>

class OpenGLShader final : public Shader
{
public:
	OpenGLShader(std::string_view sourceCode);

	void Use();

	void GetUniforms(std::vector<Uniform>& uniforms) override;

	void SetUniform(TypeInfo* type, std::string_view name, const void* data) override;

	void SetUniform(std::string_view name, const TextureHandle& texture) override;
private:
	GL::Shader m_shader;

	std::unordered_map<std::string_view, std::string> m_materialNameMap;

	template<class T>
	void SetUniform(std::string_view name, const void* address);
};

template<typename T>
void OpenGLShader::SetUniform(std::string_view name, const void* address)
{
	const T& value = *static_cast<const T*>(address);
	m_shader.SetUniform(name, value);
}
