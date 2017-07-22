#version 410 core

in vec4 TexCoords;
in vec4 FragPosLightSpace;
in vec4 FragPosViewSpace;

out vec4 FragColor;

uniform sampler2D NormalDepth;
uniform sampler2D ShadowMap;
uniform sampler2D DistanceMap;
uniform sampler2D DilatedDepthMap;

uniform vec2 direction;
uniform float lightSize;

float maxDepth = 1000.0;
float anisoThreshold = 0.25;
const int num_steps = 12-1;

const float weights[] =
float[11](
  0.209857,
  0.00556439,
  0.0222576,
  0.0612083,
  0.122417,
  0.183625,
  0.00556439,
  0.0222576,
  0.0612083,
  0.122417,
  0.183625
);

const float offsets[] = 
float[11](
  0.0,
  5.0,
  4.0,
  3.0,
  2.0,
  1.0,
  -5.0,
  -4.0, 
  -3.0,
  -2.0,
  -1.0
);

vec2 get_step_size(vec3 normal, float depth, float threshold );
float blur(float initialShadow, vec2 stepSize, float depth, float penumbraSize, vec2 TexCoordinates);

void main()
{
	vec3 projCoords = TexCoords.xyz / TexCoords.w;
	projCoords = projCoords * 0.5 + 0.5;

	vec3 DistanceData = texture(DistanceMap, projCoords.xy).rgb;

	float penumbra = DistanceData.b;

	if (penumbra ==1)
	{
		float penumbraSize = 1.0;
		penumbraSize *= DistanceData.r;
		penumbraSize *= lightSize;
		vec3 projCoordsLight = FragPosLightSpace.xyz/FragPosLightSpace.w;
		projCoordsLight = projCoordsLight * 0.5 + 0.5;
		float dBlocker = texture(DilatedDepthMap, projCoordsLight.xy).r;
		penumbraSize /= dBlocker;
		//penumbraSize /= (DistanceData.g);
		penumbraSize /= FragPosViewSpace.w;

		vec4 normAndDepth = texture(NormalDepth, projCoords.xy).rgba;
		vec2 stepSize = get_step_size(normAndDepth.rgb, DistanceData.g, anisoThreshold);
		//stepSize = vec2(0.05, 0.05);
		float curShadow = texture(ShadowMap, projCoords.xy).r;
		float shadowVal = blur(curShadow, stepSize, DistanceData.g, penumbraSize, projCoords.xy);

		FragColor = vec4(shadowVal, shadowVal, shadowVal, 1.0);
		//FragColor = vec4(1, 1, 1, 1.0);
	}
	else
	{
		FragColor = vec4(0, 0, 0, 1.0);
	}
}

vec2 get_step_size(vec3 normal, float depth, float threshold )
{
  return direction 
         //* light_size * light_size //included in the penumbra
         * sqrt( max( dot( vec3( 0, 0, 1 ), normal ), threshold ) )
         * (1 / (depth /* * depth * 100*/)) * (1.0/5.0);
}


float blur(float initialShadow, vec2 stepSize, float distance, float penumbraSize, vec2 TexCoordinates)
{
	float finalShadow = initialShadow * weights[0];
	float sumWeights = weights[0];
	
	for(int i=1; i<num_steps; i++)
	{
		vec2 sampleLoc = TexCoordinates + offsets[i] * penumbraSize * stepSize;

		float distSample = texture(DistanceMap, sampleLoc).g;
		if(true)//abs(distSample - distance) < maxDepth)
		{
			sumWeights += weights[i];
			finalShadow += texture(ShadowMap, sampleLoc).r * weights[i];
		}
	}

	finalShadow /= sumWeights;
	return finalShadow;
}