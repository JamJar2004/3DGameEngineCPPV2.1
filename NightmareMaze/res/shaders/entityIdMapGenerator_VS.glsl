#version 330

layout(location = 0) in vec3 a_position;

layout(location = 4) in mat4 u_WVPMatrix;
layout(location = 8) in uint u_entityId;

flat out uint v_entityId;

void main()
{
	gl_Position = u_WVPMatrix * vec4(a_position, 1.0);
	v_entityId = u_entityId;
}