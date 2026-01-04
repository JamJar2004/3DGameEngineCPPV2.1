#version 330

in vec2 v_position;

uniform vec3  u_color;
uniform float u_intensity;

uniform vec3 u_cameraPosition;

uniform sampler2D u_albedoTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_positionTexture;
uniform sampler2D u_specularTexture;

out vec4 o_color;

void main()
{
	vec4 texColor   = texture(u_albedoTexture, v_position);
	vec4 lightColor = vec4(u_color, 1.0) * u_intensity;
	o_color = texColor * lightColor;
}