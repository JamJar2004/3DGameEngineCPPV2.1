#include "TextureAtlas.hpp"

#include "RenderDevice.hpp"

TextureAtlas::~TextureAtlas()
{
    m_renderDevice->DeleteTexture(m_bindingIndex);
}
