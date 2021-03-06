#version 400

in vec3 LightIntensity;
in vec2 UV;
in vec4 ShadowCoord;

uniform sampler2D sampler;
uniform sampler2D shadowMap;

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

layout(location = 0) out vec4 FragColor;

void main()
{
	vec4 texel = texture(sampler, UV);
	if(texel.a < 0.1)
	{
		discard;
	}

	vec3 firstInterval = vec3(0.5, 0.5, 0.5);
	vec3 secondInterval = vec3(0.7, 0.7, 0.7);
	vec3 thirdInterval = vec3(0.9, 0.9, 0.9);
	vec3 finalColor = LightIntensity;

	/*if(LightIntensity.x <= 0.02 && LightIntensity.y <= 0.02 && LightIntensity.z <= 0.02)
	{
		finalColor = LightIntensity;
	}
	else
	{*/
		if(finalColor.r < firstInterval.r)
		{
			finalColor.r = 0.1;
		}
		else if(finalColor.r < secondInterval.r)
		{
			finalColor.r = 0.5;
		}
		else if(finalColor.r < thirdInterval.r)
		{
			finalColor.r = 0.7;
		}
		else
		{
			finalColor.r = 1.0;
		}

		if(finalColor.g < firstInterval.g)
		{
			finalColor.g = 0.1;
		}
		else if(finalColor.g < secondInterval.g)
		{
			finalColor.g = 0.5;
		}
		else if(finalColor.g < thirdInterval.g)
		{
			finalColor.g = 0.7;
		}
		else
		{
			finalColor.g = 1.0;
		}
	
		if(finalColor.b < firstInterval.b)
		{
			finalColor.b = 0.1;
		}
		else if(finalColor.b < secondInterval.b)
		{
			finalColor.b = 0.5;
		}
		else if(finalColor.b < thirdInterval.b)
		{
			finalColor.b = 0.7;
		}
		else
		{
			finalColor.b = 1.0;
		}
	//}	

	float shadowFactor = 1.0;
	float epsilon = 0.1;
	vec4 shadowCoordPD = ShadowCoord / ShadowCoord.w;
	if(ShadowCoord.w <= 0.0f)
	{
		shadowFactor = 1.0;
	}
	else if(shadowCoordPD.x < 0 || shadowCoordPD.y < 0)
	{
		shadowFactor = 1.0;
	}
	else
	{
		
		for(int i = 0; i < 16; i++)
		{
			if(texture2D(shadowMap, shadowCoordPD.xy + poissonDisk[i]/300.0).z < shadowCoordPD.z - epsilon)
			{
				shadowFactor -= 0.05;
			}
		}
	}
	FragColor = vec4(shadowFactor, shadowFactor, shadowFactor, 1.0) * vec4(finalColor, 1.0) * texel;
	//FragColor = vec4(texture(sampler, UV).rgb * LightIntensity, 1.0);
	//FragColor =  vec4(1.0, 1.0, 1.0, 1.0);
}
