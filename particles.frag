#version 400

in float alpha;

uniform sampler2D texture;

layout (location = 0) out vec4 FragColor;

void main()
{
	vec4 texel = texture2D(texture, gl_PointCoord);
	FragColor = vec4(1 - alpha, 1 - alpha, 0.75 + 0.25 * (1 - alpha), alpha) * texel;
}
