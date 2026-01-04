#version 330

in vec2 v_position;

uniform sampler2D u_source;

out vec4 o_color;

void main()
{
	o_color = texture(u_source, v_position);
}