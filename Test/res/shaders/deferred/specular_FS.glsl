#version 330

in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_worldPosition;

uniform vec3      m_Color;
uniform sampler2D m_Texture;
uniform float     m_SpecularIntensity;

uniform vec2 m_TilingFactor;

layout(location = 0) out vec4  o_albedo;
layout(location = 1) out vec3  o_normal;
layout(location = 2) out vec3  o_position;
layout(location = 3) out float o_specular;

void main()
{
	vec4 texColor = texture(m_Texture, v_texCoord * m_TilingFactor);
	
	o_albedo   = vec4(m_Color, 1.0) * texColor;
	o_normal   = v_normal;
	o_position = v_worldPosition;
	o_specular = m_SpecularIntensity;
}