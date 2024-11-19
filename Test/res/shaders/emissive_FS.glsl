#version 330

uniform vec3  m_Color;
uniform float m_Intensity;

out vec4 o_color;

void main()
{
	o_color = vec4(m_Color, 1.0) * m_Intensity;
}