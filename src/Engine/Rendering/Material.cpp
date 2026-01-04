#include "Material.hpp"

void Material::Use()
{
	for(size_t i = 0; i < m_textures.size(); i++)
	{
		TextureHandle texture = m_textures[i];
		DEBUG_ASSERT(texture, "Material cannot have null texture.");
		texture->Bind(uint32_t(i));
	}

	ShaderHandle shader = GetType();
	shader->Use(m_list->Get(m_index));
}
