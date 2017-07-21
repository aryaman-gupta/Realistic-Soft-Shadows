#version 410 core

in vec4 IPCoords;
in vec4 FragPosLightSpace;
in vec4 FragPosViewSpace;
in vec3 inNormal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D NormalDepth;
uniform sampler2D Gauss1Dilated;
uniform sampler2D ShadingBuffer;
uniform sampler2D DistanceMap;
uniform sampler2D DilatedDepthMap;
uniform sampler2D diffuseTexture;

uniform vec2 direction;
uniform float lightSize;
uniform vec3 lightPos;
uniform vec3 viewPos;

float maxDepth = 100000.0;//Needs to be set
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
	vec3 projCoords = IPCoords.xyz / IPCoords.w;
	projCoords = projCoords * 0.5 + 0.5;
	vec3 temp = texture(ShadingBuffer, projCoords.xy).rgb;

	vec3 DistanceData = texture(DistanceMap, projCoords.xy).rgb;

	float penumbra = DistanceData.b;

			
	vec3 color = texture(diffuseTexture, TexCoords).rgb;
	vec3 normal = normalize(inNormal);
	vec3 lightColor = vec3(0.3);
	// ambient
	vec3 ambient = 0.3 * color;
	// diffuse
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * lightColor;
	// specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor; 

	vec3 lighting;

	if (penumbra ==1)
	{
		float penumbraSize = 1.0;
		penumbraSize *= DistanceData.r;//DReceiver - DBlocker
		penumbraSize *= lightSize;//WLight
		vec3 projCoordsLight = FragPosLightSpace.xyz/FragPosLightSpace.w;
		projCoordsLight = projCoordsLight * 0.5 + 0.5;
		float dBlocker = texture(DilatedDepthMap, projCoordsLight.xy).r;
		penumbraSize /= dBlocker;//DBlocker
		//penumbraSize /= (DistanceData.g);
		penumbraSize /= FragPosViewSpace.w;//DObserver

		vec4 normAndDepth = texture(NormalDepth, projCoords.xy).rgba;
		vec2 stepSize = get_step_size(normAndDepth.rgb, DistanceData.g, anisoThreshold);
		//stepSize = vec2(0.05, 0.05);
		float curShadow = texture(Gauss1Dilated, projCoords.xy).r;
		float shadowVal = blur(curShadow, stepSize, DistanceData.g, penumbraSize, projCoords.xy);
		//shadowVal = 0.5f;   
        //color = vec3(0.8, 0.5, 0.5);            
		lighting = (ambient + (1.0 - shadowVal) * (diffuse + specular)) * color;
		//lighting = vec3 (shadowVal);
	}

	else
	{
		lighting = (ambient + diffuse + specular) * color;
		//lighting = vec3 (0.8, 0.5, 0.5);
	}

	FragColor = vec4(lighting, 1.0);
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
		if(abs(distSample - distance) < maxDepth)
		{
			sumWeights += weights[i];
			finalShadow += texture(Gauss1Dilated, sampleLoc).r * weights[i];
		}
	}

	finalShadow /= sumWeights;
	return finalShadow;
}