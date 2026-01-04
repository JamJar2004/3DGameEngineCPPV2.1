#version 430

struct Material
{
	vec3  color;
	float specularIntensity;
	vec2  tilingFactor;

	vec2 textureRegion;
	vec2 normalMapRegion;
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
varying mat3 v_tbnMatrix;
varying vec3 v_worldPosition;

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec3 a_tangent;

uniform vec4 u_clippingPlane;

void main()
{
	Instance instance = instances[gl_InstanceIndex];

	vec4 worldPosition = instance.modelMatrix * vec4(a_position, 1.0);
	gl_ClipDistance[0] = dot(worldPosition, u_clippingPlane);

	gl_Position = instance.mvpMatrix * vec4(a_position, 1.0);

	v_texCoord = a_texCoord;

	v_worldPosition = worldPosition.xyz;

	vec3 n = normalize((instance.modelMatrix * vec4(a_normal, 0.0)).xyz);
	vec3 t = normalize((instance.modelMatrix * vec4(a_tangent, 0.0)).xyz);
	t = normalize(t - dot(t, n) * n);

	vec3 biTangent = cross(t, n);
	v_tbnMatrix = mat3(t, biTangent, n);
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4  o_albedo;
layout(location = 1) out vec3  o_normal;
layout(location = 2) out vec3  o_position;
layout(location = 3) out float o_specular;

void main()
{
	Instance instance = instances[gl_InstanceIndex];

	vec2 texCoord = v_texCoord * instance.material.tilingFactor;

	vec4 texColor = texture(m_Texture, texCoord);

	vec4 normalMapColor = texture(m_NormalMap, texCoord);

	vec3 normal = normalize(v_tbnMatrix * (255.0 / 128.0 * normalMapColor.xyz - 1.0));

	o_albedo   = vec4(instance.material.color, 1.0) * texColor;
	o_normal   = normal;
	o_position = v_worldPosition;
	o_specular = instance.material.specularIntensity;
}

#endif