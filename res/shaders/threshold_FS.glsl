#version 330

in vec2 v_position;

uniform sampler2D u_source;
uniform float u_threshold;
uniform int u_levels;

out vec4 o_color;

void main()
{
	vec3 color = texture(u_source, v_position).rgb;
	if(color.r < u_threshold && color.g < u_threshold && color.b < u_threshold)
		color = vec3(0.0, 0.0, 0.0);

	color /= u_levels;
	o_color = vec4(color, 1.0);
}