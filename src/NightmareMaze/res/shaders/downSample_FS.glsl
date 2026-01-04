#version 330

in vec2 v_position;

uniform sampler2D u_source;
uniform vec2      u_sourceSize;

out vec4 o_color;

void main()
{
	vec2 sourcePixelSize = 1.0 / u_sourceSize;

	vec2 halfPixelSize = sourcePixelSize * 0.5;

	vec4 tl = texture(u_source, v_position - halfPixelSize);
	vec4 tr = texture(u_source, v_position - halfPixelSize + vec2(sourcePixelSize.x, 0.0));
	vec4 bl = texture(u_source, v_position - halfPixelSize + vec2(0.0, sourcePixelSize.y));
	vec4 br = texture(u_source, v_position - halfPixelSize + vec2(sourcePixelSize.x, sourcePixelSize.y));


	o_color = (tl + tr + bl + br) / 4.0;
}