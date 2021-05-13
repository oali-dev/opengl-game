#version 330 core

layout(location = 0) in vec3 VertexPosition;

out vec2 UV;

void main(){
	gl_Position = vec4(VertexPosition, 1);
	UV = (VertexPosition.xy + vec2(1, 1)) / 2.0;
}

