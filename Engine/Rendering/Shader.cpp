#include "Shader.hpp"

void Shader::SetTexture(const std::string& name, TextureHandle texture)
{
	uint32_t slot = GetTextureSlot(name);
	m_textures[slot] = texture;
}

bool Shader::TrySetTexture(const std::string& name, TextureHandle texture)
{
	auto it = m_textureMap.find(name);
	if(it == m_textureMap.end())
		return false;

	uint32_t index = it->second;
	m_textures[index] = texture;
	return true;
}

void Shader::Use(const void* materialData)
{
	for(auto it = m_textures.begin(); it != m_textures.end(); ++it)
	{
		TextureHandle texture = it->second;
		DEBUG_ASSERT(texture, "Material cannot have null texture.");
		texture->Bind(it->first);
	}

	OnUse(materialData);
}
