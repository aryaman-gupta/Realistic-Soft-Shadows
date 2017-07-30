#version 410 core

in vec4 TexCoords;
in vec4 FragPosLightSpace;
in vec4 FragPosViewSpace;

out vec3 FragColor;

uniform sampler2D NormalDepth;
uniform sampler2D ShadowMap;
uniform sampler2D DistanceMap;
uniform sampler2D DilatedDepthMap;

uniform float lightSize;

float anisoThreshold = 0.25;

vec2 get_step_size(vec3 normal, float depth, float threshold );


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

		//float curShadow = texture(ShadowMap, projCoords.xy).r;
		//float shadowVal = blur(curShadow, stepSize, DistanceData.g, penumbraSize, projCoords.xy);

		FragColor = vec3(penumbraSize, stepSize);
	}
	else
	{
		FragColor = vec3(0.0, 0.0, 0.0);
	}
}

vec2 get_step_size(vec3 normal, float depth, float threshold )
{
  return sqrt( max( dot( vec3( 0, 0, 1 ), normal ), threshold ) ) * vec2(1, 1)
         * (1 / (depth /* * depth * 100*/)) * (1.0/5.0);
         //* light_size * light_size //included in the penumbra
}