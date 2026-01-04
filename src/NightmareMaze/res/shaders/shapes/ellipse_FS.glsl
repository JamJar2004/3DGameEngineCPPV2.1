#version 330

in vec2 v_position;

flat in vec2  v_scale;
flat in float v_borderThickness;
flat in vec4  v_borderColor;
flat in vec4  v_fillColor;

out vec4 o_color;

uniform vec2 u_screenSize;

void main()
{
	vec2 pixelPosition = v_position * v_scale;
	vec2 innerPosition = (pixelPosition - v_borderThickness) / (v_scale - v_borderThickness * 2.0);

	float outerDistanceFromCentre = length(v_position - vec2(0.5));
	float innerDistanceFromCentre = length(innerPosition - vec2(0.5));

	if(outerDistanceFromCentre <= 0.5)
	{
		if(innerDistanceFromCentre <= 0.5)
		{
			o_color = v_fillColor;
		}
		else
		{
			o_color = v_borderColor;
		}
	}	   
	else
	{
		o_color = vec4(0.0, 0.0, 0.0, 0.0);
	}
}