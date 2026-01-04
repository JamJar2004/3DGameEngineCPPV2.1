#version 330

in vec2 v_position;

uniform sampler2D u_source;
uniform vec2      u_sourceSize;

out vec4 o_color;

void main()
{
	const int blurFactor = 8;
	const int halfBlurFactor = blurFactor / 2;
	
	const int blurArea = blurFactor * blurFactor;

	vec2 sourcePixelSize = 1.0 / u_sourceSize;

	vec3 resultColor = vec3(0.0, 0.0, 0.0);
	for(int i = -halfBlurFactor; i <= halfBlurFactor; i++)
	{
		for(int j = -halfBlurFactor; j <= halfBlurFactor; j++)
		{
			float dist = length(vec2(i, j)) / blurArea;
			
			float weight = 1.0 - dist;
		
			vec3 color = texture(u_source, v_position + (vec2(i, j) * sourcePixelSize)).xyz;
			resultColor += color * weight;
		}
	}

	o_color = vec4(resultColor / blurArea, 1.0);
}