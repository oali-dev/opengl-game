#version 400

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexUVCoords;
layout (location = 2) in vec3 VertexNormal;
layout (location = 3) in ivec4 BoneIDs;
layout (location = 4) in vec4 BoneWeights;

out vec3 LightIntensity;
out vec2 UV;
out vec4 ShadowCoord;

uniform vec4 LightPosition;
uniform vec3 Ka;			
uniform vec3 Kd;			
uniform vec3 Ks;
uniform float shininess;
uniform vec3 La;			
uniform vec3 Ld;			
uniform vec3 Ls;			
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;
uniform mat4 boneMatrices[64];
uniform bool isDynamic;
uniform mat4 depthBiasMVP;

void main()
{
	bool d = true;
	mat4 boneMatrix;
	if(isDynamic)
	{
		boneMatrix = boneMatrices[BoneIDs[0]] * BoneWeights[0];
		boneMatrix += boneMatrices[BoneIDs[1]] * BoneWeights[1];	
		boneMatrix += boneMatrices[BoneIDs[2]] * BoneWeights[2];	
		boneMatrix += boneMatrices[BoneIDs[3]] * BoneWeights[3];	
	}
	else
	{
		boneMatrix = mat4(1.0f);
	}

	vec3 tnorm = normalize( NormalMatrix * mat3(boneMatrix) * VertexNormal);
	vec4 eyeCoords = ModelViewMatrix * vec4(VertexPosition, 1.0);
	vec3 s = normalize(vec3(LightPosition - eyeCoords));
	vec3 v = normalize(-eyeCoords.xyz);
	vec3 r = reflect(-s, tnorm);
	vec3 ambient = La * Ka;
	float sDotN = max(dot(s, tnorm), 0.0);
	vec3 diffuse = Ld * Kd * sDotN;
	vec3 spec = vec3(0.0);
	
	if(sDotN > 0.0)
	{
		spec = Ls * Ks * pow(max(dot(r, v), 0.0), shininess);
	}

	LightIntensity = ambient + diffuse + spec;
	float nDotV = dot(tnorm, v);
	/*if(nDotV < 0.08)
	{
		LightIntensity = vec3(0.0, 0.0, 0.0);
	}*/

	UV = VertexUVCoords; //* 5;
	vec4 boneSpacePosition = boneMatrix * vec4(VertexPosition, 1.0);
	ShadowCoord = depthBiasMVP * boneSpacePosition;
	gl_Position = MVP * boneSpacePosition;
}
