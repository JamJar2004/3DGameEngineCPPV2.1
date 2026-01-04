#version 330

in vec3 v_texCoord;

uniform samplerCube m_CubeMap;
uniform float m_Emission;

out vec4 o_color;

void main()
{
	o_color = texture(m_CubeMap, v_texCoord) * m_Emission;
}