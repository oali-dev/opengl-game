#version 400

layout (location = 0) in vec3 Velocity;
layout (location = 1) in vec3 ParticlePosition;

uniform mat4 viewProjMatrix; // projectionMatrix * viewMatrix
uniform vec3 position;
uniform float curTime;
uniform float startTime;

const vec3 gravity = vec3(0.0, -9.81, 0.0);

out float alpha;

void main()
{
	float time = curTime - startTime;
	alpha = (1 - (time / 3) - 0.2);
	vec3 finalPosition;

	if(time > 0)
		finalPosition = (position + ParticlePosition) + Velocity * time + 0.5 * gravity * pow(time, 2);
	else
		finalPosition = position + ParticlePosition;
	gl_Position = viewProjMatrix * vec4(finalPosition, 1.0);
}
