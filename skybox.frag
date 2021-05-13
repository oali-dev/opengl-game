#version 330

in vec3 UV;
out vec4 FragColor;

uniform samplerCube CubeMap;

void main()
{
	FragColor = texture(CubeMap, UV);
}
