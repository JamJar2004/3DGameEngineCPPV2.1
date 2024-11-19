#version 330

in vec2 v_texCoord;
in mat3 v_tbnMatrix;
in vec3 v_worldPosition;

uniform vec3      m_Color;
uniform sampler2D m_Texture;
uniform sampler2D m_NormalMap;
uniform float     m_SpecularIntensity;

uniform vec2 m_TilingFactor;

layout(location = 0) out vec4  o_albedo;
layout(location = 1) out vec3  o_normal;
layout(location = 2) out vec3  o_position;
layout(location = 3) out float o_specular;

void main()
{

	vec4 texColor = texture(m_Texture, sampleLocation * m_TilingFactor);
	
	vec4 normalMapColor = texture(m_NormalMap, v_texCoord * m_TilingFactor);
	
	vec3 normal = normalize(v_tbnMatrix * (255.0 / 128.0 * normalMapColor.xyz - 1.0));
	
	o_albedo   = vec4(m_Color, 1.0) * texColor;
	o_normal   = normal;
	o_position = v_worldPosition;
	o_specular = m_SpecularIntensity;
}