#version 330

in vec2 v_position;
in vec3 v_lightDirection;
in mat4 v_lightMatrix;

uniform vec3  u_color;
uniform float u_intensity;
uniform float u_shadowSoftness;

uniform vec3 u_cameraPosition;

uniform sampler2D u_normalTexture;
uniform sampler2D u_positionTexture;
uniform sampler2D u_specularTexture;

uniform sampler2D u_shadowMap;
uniform vec2 u_shadowMapSize;

out vec4 o_color;

float SampleShadowMap(sampler2D shadowMap, vec2 coords, float compare, float shadowBias)
{
	if(texture(shadowMap, coords).r >= compare - shadowBias)
		return 1.0;
		
	return 0.0;
}

float SampleShadowMapLinear(sampler2D shadowMap, vec2 coords, float compare, vec2 pixelSize, float shadowBias)
{
	vec2 pixelPosition = coords / pixelSize + 0.5;
	vec2 fractionalPart = fract(pixelPosition);
	vec2 startPixel = (pixelPosition - fractionalPart) * pixelSize;

	float bl = SampleShadowMap(shadowMap, startPixel, compare, shadowBias);
	float br = SampleShadowMap(shadowMap, startPixel + vec2(pixelSize.x, 0), compare, shadowBias);
	float tl = SampleShadowMap(shadowMap, startPixel + vec2(0, pixelSize.y), compare, shadowBias);
	float tr = SampleShadowMap(shadowMap, startPixel + pixelSize, compare, shadowBias);

	float bottom = mix(bl, br, fractionalPart.x);
	float top    = mix(tl, tr, fractionalPart.x);

	return mix(bottom, top, fractionalPart.y);
}

float SampleShadowMapPCF(sampler2D shadowMap, vec2 coords, float compare, vec2 pixelSize, float samples)
{
	float samplesStart = (samples - 1.0) * 0.5;

	float result = 0.0;

	for(float y = -samplesStart; y <= samplesStart; y += 1.0)
	{
		for(float x = -samplesStart; x <= samplesStart; x += 1.0)
		{
			vec2 coordsOffset = vec2(x, y) * pixelSize;
			result += SampleShadowMapLinear(shadowMap, coords + coordsOffset, compare, pixelSize, samples * 0.001);
		}
	}

	return result / (samples * samples);
}

float CalcSpecularFactor(vec3 lightDirection, vec3 surfaceNormal, vec3 worldPosition, vec3 cameraPosition, float specularIntensity)
{
	vec3 unitDirection = normalize(lightDirection);
	vec3 unitNormal    = normalize(surfaceNormal);

    vec3 directionToEye = normalize(cameraPosition - worldPosition);
    vec3 reflectedDirection = normalize(reflect(unitDirection, unitNormal));

    float specularFactor = dot(directionToEye, reflectedDirection);
	if(specularFactor < 0.0)
		return 0.0;
	
    specularFactor = pow(specularFactor, specularIntensity * 8.0) * specularIntensity;

    return max(specularFactor, 0.0);
}

void main()
{
	vec3 normal        = texture(u_normalTexture, v_position).xyz;
	vec3 worldPosition = texture(u_positionTexture, v_position).xyz;
	
	vec4 transformedShadowMapCoords = v_lightMatrix * vec4(worldPosition, 1.0);
	vec3 shadowMapCoords = (transformedShadowMapCoords.xyz / transformedShadowMapCoords.w) * 0.5 + 0.5;
	
	float shadowFactor = SampleShadowMapPCF(u_shadowMap, shadowMapCoords.xy, shadowMapCoords.z, 1.0 / u_shadowMapSize, u_shadowSoftness);
	
	float specularIntensity = texture(u_specularTexture, v_position).r;

	float specularFactor = CalcSpecularFactor(v_lightDirection, normal, worldPosition.xyz, u_cameraPosition, specularIntensity);

	vec4 lightColor = vec4(u_color, 1.0) * u_intensity;
	
	o_color = (lightColor * specularFactor) * shadowFactor;
}