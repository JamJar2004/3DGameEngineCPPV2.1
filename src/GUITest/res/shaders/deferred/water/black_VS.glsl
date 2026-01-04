#version 330

layout(location = 0) in vec2 a_position;

layout(location = 5) in mat4 u_WVPMatrix;

void main()
{
	gl_Position = u_WVPMatrix * vec4(a_position.x, 0.0, a_position.y, 1.0);;
}
