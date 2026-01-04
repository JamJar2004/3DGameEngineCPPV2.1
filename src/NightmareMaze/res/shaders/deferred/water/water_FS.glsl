#version 330

in vec4 v_projectedTexCoords;
in vec3 v_worldPosition;
in vec2 v_texCoord;

uniform sampler2D m_DistortionMap;
uniform sampler2D m_NormalMap;
uniform vec2      m_TilingFactor;
uniform float     m_WaveStrength;
uniform float     m_SpecularIntensity;

uniform float u_offset;

uniform vec3 u_cameraPosition;

uniform sampler2D u_reflection;
uniform sampler2D u_refraction;
uniform sampler2D u_depthTexture;

layout(location = 0) out vec4  o_albedo;
layout(location = 1) out vec3  o_normal;
layout(location = 2) out vec3  o_position; 
layout(location = 3) out float o_specular;

void main()
{
	vec2 projectedTexCoords = (v_projectedTexCoords.xy / v_projectedTexCoords.w) * 0.5 + 0.5;

	vec2 tiledTexCoords = v_texCoord * m_TilingFactor;

	float near = 0.1;
	float far = 1000.0;

	float depth = texture(u_depthTexture, projectedTexCoords).r;
	float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

	depth = gl_FragCoord.z;
	float waterDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
	float waterDepth = floorDistance - waterDistance;

	float depthFactor = clamp(waterDepth / 5.0, 0.0, 1.0);

	vec2 distortion1 = texture(m_DistortionMap, tiledTexCoords + u_offset).rg * 2.0 - 1.0;
	vec2 distortion2 = texture(m_DistortionMap, tiledTexCoords - vec2(u_offset, -u_offset * 0.4)).rg * 2.0 - 1.0;

	vec2 distortions = (distortion1 + distortion2) * m_WaveStrength * depthFactor;

	vec2 reflectionTexCoords = vec2(projectedTexCoords.x, 1.0 - projectedTexCoords.y) + distortions;
	reflectionTexCoords = clamp(reflectionTexCoords, 0.001, 0.999);

	vec2 refractionTexCoords = projectedTexCoords + distortions;
	refractionTexCoords = clamp(refractionTexCoords, 0.001, 0.999);

	vec4 reflectionColor = texture(u_reflection, reflectionTexCoords);
	vec4 refractionColor = texture(u_refraction, refractionTexCoords);
	
	vec3 toCameraVector = u_cameraPosition - v_worldPosition;

	vec4 normalMapColor = texture(m_NormalMap, tiledTexCoords + u_offset + distortions);
	
	vec3 normal = normalize(vec3((normalMapColor.r * 2.0 - 1.0) * depthFactor, normalMapColor.b / (m_WaveStrength * 20.0), (normalMapColor.g * 2.0 - 1.0) * depthFactor));

	float fresnelEffect = dot(normalize(toCameraVector), normal);
	
	float refractiveFactor = mix(fresnelEffect, 1.0 - depthFactor, 0.25);
	o_albedo = mix(reflectionColor, refractionColor, refractiveFactor);
	o_normal = normal;
	o_position = v_worldPosition;
	o_specular = m_SpecularIntensity * depthFactor;
}