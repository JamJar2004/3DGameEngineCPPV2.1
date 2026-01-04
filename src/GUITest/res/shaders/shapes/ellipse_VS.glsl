#version 330

layout(location = 0) in vec2 a_position;

layout(location = 1) in vec2  u_offset;
layout(location = 2) in vec2  u_scale;
layout(location = 3) in float u_borderThickness;
layout(location = 4) in vec4  u_borderColor;
layout(location = 5) in vec4  u_fillColor;

out vec2 v_position;

flat out vec2  v_scale;
flat out float v_borderThickness;
flat out vec4  v_borderColor;
flat out vec4  v_fillColor;

uniform vec2 u_screenSize;

void main()
{
	vec2 invertedOffset = u_offset;
	invertedOffset.y = u_screenSize.y - u_offset.y - u_scale.y;
	
	vec2 normalizedOffset = invertedOffset / u_screenSize;
	vec2 normalizedScale  = u_scale        / u_screenSize;

	gl_Position = vec4(a_position * normalizedScale + (normalizedOffset * 2.0) - 1.0 + normalizedScale, 0.0, 1.0);
	
	v_position        = a_position * 0.5 + 0.5;
	v_scale           = u_scale;
	v_borderThickness = u_borderThickness;
	v_borderColor     = u_borderColor;
	v_fillColor       = u_fillColor;
}