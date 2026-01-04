#version 330

layout(location = 0) in vec2 a_position;

layout(location = 1) in mat4 u_worldMatrix;
layout(location = 5) in mat4 u_WVPMatrix;

out vec4 v_projectedTexCoords;
out vec3 v_worldPosition;
out vec2 v_texCoord;

void main()
{
	v_projectedTexCoords = u_WVPMatrix * vec4(a_position.x, 0.0, a_position.y, 1.0);
	v_worldPosition      = (u_worldMatrix * vec4(a_position.x, 0.0, a_position.y, 1.0)).xyz;
	v_texCoord           = a_position * 0.5 + 0.5;
	
	gl_Position = v_projectedTexCoords;
}
