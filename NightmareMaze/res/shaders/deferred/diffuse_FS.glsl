#version 330

in vec2 v_texCoord;
in vec3 v_normal;

uniform vec3 m_Color;
uniform sampler2D m_Texture;

uniform vec2 m_CellOffset;
uniform vec2 m_CellSize;

uniform vec2 m_TilingFactor;

layout(location = 0) out vec4 o_albedo;
layout(location = 1) out vec3 o_normal;

void main()
{
	vec4 texColor = texture(m_Texture, (v_texCoord * m_CellSize + m_CellOffset) * m_TilingFactor);
	o_albedo = vec4(m_Color, 1.0) * texColor;
	o_normal = v_normal;
}