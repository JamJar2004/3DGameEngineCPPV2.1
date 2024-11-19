#version 330

layout(location = 0) in vec2 a_position;

layout(location = 1) in vec3 u_lightDirection;
layout(location = 2) in vec3 u_lightPosition;

out vec2 v_position;
out vec3 v_lightPosition;

void main()
{
	gl_Position = vec4(a_position, 0.0, 1.0);
	v_position = a_position * 0.5 + 0.5;
	v_lightPosition = u_lightPosition;
}