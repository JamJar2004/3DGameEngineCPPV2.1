#version 330

in vec2 v_position;

flat in vec2  v_atlasOffset;
flat in vec2  v_size;
flat in float v_thickness;
flat in float v_borderThickness;
flat in vec4  v_borderColor;
flat in vec4  v_fillColor;

out vec4 o_color;

uniform sampler2D u_textureAtlas;

uniform vec2 u_atlasSize;
uniform vec2 u_screenSize;

void main()
{
	vec2 sampleLocation = vec2(v_position.x, 1.0 - v_position.y) * (v_size / u_atlasSize) + (v_atlasOffset / u_atlasSize);
	float distanceValue = 1.0 - texture(u_textureAtlas, sampleLocation).r;
	
	//o_color = mix(vec4(0.0, 0.0, 0.0, 0.0), v_borderColor, smoothstep(1.0 - v_borderThickness - 0.2, 1.0 - v_borderThickness, distanceValue));
	
	float borderThickness = (1.0 - v_borderThickness) * v_thickness;
	
	o_color = mix(v_fillColor, v_borderColor, smoothstep(borderThickness, borderThickness + 0.1, distanceValue));
	o_color.a *= (1.0 - smoothstep(v_thickness, v_thickness + 0.1, distanceValue));
	
	//if(distanceValue < v_thickness)
	//{
	//	
	//	if(distanceValue > 1.0 - v_borderThickness)
	//	{
	//		o_color = v_borderColor;
	//	}
	//	else
	//	{
	//		o_color = v_fillColor;
	//	}
	//}
	//else
	//{
	//	o_color = vec4(0.0, 0.0, 0.0, 0.0);
	//}
}