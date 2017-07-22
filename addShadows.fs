#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform int numLights;
uniform sampler2D lightShadowMaps[10];

void main()
{
	float finalShadow = 0;
	for(int i = 0; i<numLights; i++)
	{
		float temp = texture (lightShadowMaps[i], TexCoords).r;
		finalShadow += temp;
	}

	if (finalShadow > 1)
		finalShadow = 1;

	FragColor = vec4(finalShadow, 0, 0, 1);
}