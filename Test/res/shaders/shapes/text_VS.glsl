#version 330

layout(location = 0) in vec2 a_position;

layout(location = 1) in vec2  u_atlasOffset;
layout(location = 2) in vec2  u_offset;
layout(location = 3) in vec2  u_size;
layout(location = 4) in vec2  u_origin;
layout(location = 5) in float u_thickness;
layout(location = 6) in float u_borderThickness;
layout(location = 7) in vec4  u_borderColor;
layout(location = 8) in vec4  u_fillColor;

out vec2 v_position;

flat out vec2  v_atlasOffset;
flat out vec2  v_size;
flat out float v_thickness;
flat out float v_borderThickness;
flat out vec4  v_borderColor;
flat out vec4  v_fillColor;

uniform float u_scale;

uniform vec2 u_screenSize;

void main()
{
	vec2 normalizedOffset = u_offset / u_screenSize;
	normalizedOffset.y = 1.0 - normalizedOffset.y;
	
	vec2 normalizedOrigin = (u_origin / u_screenSize) * u_scale;
	vec2 normalizedScale  = (u_size / u_screenSize) * u_scale;

	gl_Position = vec4(a_position * normalizedScale + normalizedOffset * 2.0 + normalizedOrigin - 1.0, 0.0, 1.0);

	v_position = a_position * 0.5 + 0.5;
	
	v_atlasOffset     = u_atlasOffset;
	v_size            = u_size;
	v_thickness       = u_thickness;
	v_borderThickness = u_borderThickness;
	v_borderColor     = u_borderColor;
	v_fillColor       = u_fillColor;
}