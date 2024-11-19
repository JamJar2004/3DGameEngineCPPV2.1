#version 330

layout(location = 0) in vec2 a_position;

layout(location = 1) in vec2 u_offset;
layout(location = 2) in vec2 u_scale;
layout(location = 3) in float u_borderThickness;

layout(location = 4) in vec4 u_borderColor;
layout(location = 5) in vec4 u_fillColor;

out vec2 v_position;

flat out float v_borderThickness;
flat out vec4  v_borderColor;
flat out vec4  v_fillColor;

uniform vec2 u_screenSize;

void main()
{
	gl_Position = vec4(a_position * (u_scale / u_screenSize) + (u_offset / u_screenSize), 0.0, 1.0);
	
	v_position        = a_position * 0.5 + 0.5;
	v_borderThickness = u_borderThickness;
	v_borderColor     = u_borderColor;
	v_fillColor       = u_fillColor;
}