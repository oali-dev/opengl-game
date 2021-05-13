#version 330

in vec3 VertexPosition;
out vec3 UV;

uniform mat4 MVP;

void main()
{
	vec4 position = MVP * vec4(VertexPosition, 1.0);
	gl_Position = position.xyww;
	UV = VertexPosition;
}
