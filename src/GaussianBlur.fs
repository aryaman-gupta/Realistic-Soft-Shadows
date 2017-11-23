#version 410 core

in vec4 TexCoords;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform sampler2D ShadowMap;
uniform sampler2D PenumbraSizeMap;
uniform sampler2D DistanceMap;

uniform vec2 direction;

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

float blur(float initialShadow, vec2 stepSize, float distance, float penumbraSize, vec2 TexCoordinates);

void main()
{
	vec3 projCoords = TexCoords.xyz / TexCoords.w;
	projCoords = projCoords * 0.5 + 0.5;

	vec3 penumbraData = texture(PenumbraSizeMap, projCoords.xy).rgb;

	float penumbraSize = penumbraData.r;
	vec2 stepSize = penumbraData.gb * direction;

	float curShadow = texture(ShadowMap, projCoords.xy).r;
	vec3 DistanceData = texture(DistanceMap, projCoords.xy).rgb;
	float distance = DistanceData.g;

	float checkPenumbra = DistanceData.b;

		
	if(penumbraSize >= 0.0)
	{
		float shadowVal = blur(curShadow, stepSize, distance, penumbraSize, projCoords.xy);
		FragColor = vec4(shadowVal, shadowVal, shadowVal, 1.0);
		//FragColor = vec4(1.0);
	}

	else
	{
		FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
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