#version 330 core

in vec2 UV;

out vec3 color;

uniform sampler2D renderedTexture;
//uniform float time;

void main()
{
	vec3 w = vec3(0.2125, 0.7154, 0.0721);

	ivec2 resolution = textureSize(renderedTexture, 0);
	float width = float(resolution.s);
	float height = float(resolution.t);

	vec2 offsetWidth = vec2(1.0 / width, 0.0);
	vec2 offsetHeight = vec2(0.0, 1.0 / height);
	vec2 offsetCornerBR = vec2(1.0 / width, 1.0 / height);
	vec2 offsetCornerTR = vec2(1.0 / width, -1.0 / height);

	float topL = dot(texture(renderedTexture, UV - offsetCornerBR).rgb, w);
	float botR = dot(texture(renderedTexture, UV + offsetCornerBR).rgb, w);
	float botL = dot(texture(renderedTexture, UV - offsetCornerTR).rgb, w);
	float topR = dot(texture(renderedTexture, UV + offsetCornerTR).rgb, w);
	float midL = dot(texture(renderedTexture, UV - offsetWidth).rgb, w);
	float midR = dot(texture(renderedTexture, UV + offsetWidth).rgb, w);
	float topM = dot(texture(renderedTexture, UV - offsetHeight).rgb, w);
	float botM = dot(texture(renderedTexture, UV + offsetHeight).rgb, w);

	float h = -botL - 2.0 * botM - botR + topL + 2.0 * topM + topR;
	float v = -topL - 2.0 * midL - botL + topR + 2.0 * midR + botR;
	float mag = length(vec2(h, v));

	if(mag > 0.97)
	{
		color = vec3(0.0, 0.0, 0.0);
	}
	else
	{
		discard;
	}
}
