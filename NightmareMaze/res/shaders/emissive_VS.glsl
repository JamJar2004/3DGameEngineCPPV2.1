#version 330

layout(location = 0) in vec3 a_position;

layout(location = 4) in mat4 u_WVPMatrix;

void main()
{
	gl_Position = u_WVPMatrix * vec4(a_position, 1.0);
}