#version 330

in vec2 v_position;
in vec3 v_lightPosition;

uniform vec3  u_color;
uniform float u_intensity;

uniform float u_constant;
uniform float u_linear;
uniform float u_exponent;
uniform float u_range;

//uniform vec3 u_position;

uniform vec3 u_cameraPosition;

uniform sampler2D u_normalTexture;
uniform sampler2D u_positionTexture;
uniform sampler2D u_specularTexture;

out vec4 o_color;

float CalcSpecularFactor(vec3 lightDirection, vec3 surfaceNormal, vec3 worldPosition, vec3 cameraPosition, float specularIntensity)
{
	vec3 unitDirection = normalize(lightDirection);
	vec3 unitNormal    = normalize(surfaceNormal);

    vec3 directionToEye = normalize(cameraPosition - worldPosition);
    vec3 reflectedDirection = normalize(reflect(unitDirection, unitNormal));

    float specularFactor = dot(directionToEye, reflectedDirection);
	if(specularFactor < 0.0)
		return 0.0;
	
    specularFactor = pow(specularFactor, specularIntensity * 4.0) * specularIntensity;

    return max(specularFactor, 0.0);
}

float calcAttenuation(float distanceToPoint, float constant, float linear, float exponent)
{
    float attenuation = constant + linear * distanceToPoint + exponent * distanceToPoint * distanceToPoint + 0.0001;
    return attenuation;
}

void main()
{
	vec3 normal        = texture(u_normalTexture, v_position).xyz;
	vec3 worldPosition = texture(u_positionTexture, v_position).xyz;
	
	float specularIntensity = texture(u_specularTexture, v_position).r;

	vec3 lightDirection = worldPosition - v_lightPosition;
	float distanceToPoint = length(lightDirection);

	if(distanceToPoint > u_range)
		discard;

	lightDirection = normalize(lightDirection);

	float diffuseFactor  = max(dot(-lightDirection, normal), 0.0);
	float specularFactor = CalcSpecularFactor(lightDirection, normal, worldPosition, u_cameraPosition, specularIntensity);

	vec4 lightColor = vec4(u_color, 1.0) * u_intensity;
	
	float attenuation = calcAttenuation(distanceToPoint, u_constant, u_linear, u_exponent);
	
	o_color = (lightColor * specularFactor) / attenuation;
}