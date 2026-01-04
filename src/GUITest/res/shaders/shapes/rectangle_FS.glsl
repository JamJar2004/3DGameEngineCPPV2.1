#version 330

in vec2 v_position;

flat in vec2  v_scale;
flat in float v_borderThickness;
flat in float v_cornerRadius;
flat in vec4  v_borderColor;
flat in vec4  v_fillColor;

out vec4 o_color;

uniform vec2 u_screenSize;

void main()
{
	vec2 pixelPosition = v_position * v_scale;

	if(pixelPosition.x <= v_cornerRadius && pixelPosition.y <= v_cornerRadius)
	{
		vec2 position = pixelPosition / v_cornerRadius;
		float distanceFromPivot = length(1.0 - position);
		if(distanceFromPivot <= 1.0)
		{
			if(distanceFromPivot <= 1.0 - (v_borderThickness / v_cornerRadius))
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
	else if(pixelPosition.x >= (v_scale.x - v_cornerRadius) && pixelPosition.y <= v_cornerRadius)
	{
		vec2 position = (pixelPosition - vec2(v_scale.x - v_cornerRadius, 0.0)) / v_cornerRadius;
		position.y = 1.0 - position.y;
		float distanceFromPivot = length(position);
		if(distanceFromPivot <= 1.0)
		{
			if(distanceFromPivot <= 1.0 - (v_borderThickness / v_cornerRadius))
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
	else if(pixelPosition.x <= v_cornerRadius && pixelPosition.y >= (v_scale.y - v_cornerRadius))
	{
		vec2 position = (pixelPosition - vec2(0.0, v_scale.y - v_cornerRadius)) / v_cornerRadius;
		position.x = 1.0 - position.x;
		float distanceFromPivot = length(position);
		if(distanceFromPivot <= 1.0)
		{
			if(distanceFromPivot <= 1.0 - (v_borderThickness / v_cornerRadius))
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
	else if(pixelPosition.x >= (v_scale.x - v_cornerRadius) && pixelPosition.y >= (v_scale.y - v_cornerRadius))
	{
		vec2 position = (pixelPosition - vec2(v_scale.x - v_cornerRadius, v_scale.y - v_cornerRadius)) / v_cornerRadius;
		float distanceFromPivot = length(position);
		if(distanceFromPivot <= 1.0)
		{
			if(distanceFromPivot <= 1.0 - (v_borderThickness / v_cornerRadius))
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
	else
	{
		if(pixelPosition.x <= v_borderThickness ||
		   pixelPosition.y <= v_borderThickness ||
		   pixelPosition.x >= v_scale.x - v_borderThickness ||
		   pixelPosition.y >= v_scale.y - v_borderThickness)
		{
			o_color = v_borderColor;
		}	   
		else
		{
			o_color = v_fillColor;
		}
	}
}