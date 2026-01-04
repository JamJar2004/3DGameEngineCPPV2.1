#version 430

struct Material
{
	vec3 color;
	float specularIntensity;
	vec2 tilingFactor;

	vec2 textureRegion;
};

struct Instance
{
	mat4 modelMatrix;
	mat4 mvpMatrix;
	Material material;
};

layout(std430, binding = 3) buffer instanceBuffer
{
	Instance instances[];
};

varying vec2 v_texCoord;
varying vec3 v_normal;
varying vec3 v_worldPosition;

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec3 a_normal;

layout(location = 4) in mat4 u_worldMatrix;
layout(location = 8) in mat4 u_WVPMatrix;

uniform vec4 u_clippingPlane;

void main()
{
	vec4 worldPosition = u_worldMatrix * vec4(a_position, 1.0);
	gl_ClipDistance[0] = dot(worldPosition, u_clippingPlane);

	gl_Position = u_WVPMatrix * vec4(a_position, 1.0);

	v_texCoord      = a_texCoord;
	v_normal        = normalize((u_worldMatrix * vec4(a_normal, 0.0)).xyz);
	v_worldPosition = worldPosition.xyz;
}

#endif

#ifdef FRAGMENT_SHADER

uniform vec3      m_Color;
uniform sampler2D m_Texture;
uniform float     m_SpecularIntensity;

uniform vec2 m_TilingFactor;

layout(location = 0) out vec4  o_albedo;
layout(location = 1) out vec3  o_normal;
layout(location = 2) out vec3  o_position;
layout(location = 3) out float o_specular;

void main()
{
	vec4 texColor = texture(m_Texture, v_texCoord * m_TilingFactor);

	o_albedo   = vec4(m_Color, 1.0) * texColor;
	o_normal   = v_normal;
	o_position = v_worldPosition;
	o_specular = m_SpecularIntensity;
}

#endif