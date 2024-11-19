#version 330

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec3 a_normal;

layout(location = 4) in mat4 u_worldMatrix;
layout(location = 8) in mat4 u_WVPMatrix;

uniform vec4 u_clippingPlane;

out vec2 v_texCoord;
out vec3 v_normal;
out vec3 v_worldPosition;

void main()
{
	vec4 worldPosition = u_worldMatrix * vec4(a_position, 1.0);
	gl_ClipDistance[0] = dot(worldPosition, u_clippingPlane);

	gl_Position = u_WVPMatrix * vec4(a_position, 1.0);
	v_texCoord      = a_texCoord;
	v_normal        = normalize((u_worldMatrix * vec4(a_normal, 0.0)).xyz);
	v_worldPosition = worldPosition.xyz;
}