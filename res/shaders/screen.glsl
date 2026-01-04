
varying vec2 v_position;

#ifdef VERTEX_SHADER
layout(location = 0) in vec2 a_position;

void main()
{
	gl_Position = vec4(a_position, 0.0, 1.0);
	v_position = a_position * 0.5 + 0.5;
}
#endif

#ifdef FRAGMENT_SHADER

uniform sampler2D u_source;

out vec4 o_color;

void main()
{
	o_color = texture(u_source, v_position);
}
#endif