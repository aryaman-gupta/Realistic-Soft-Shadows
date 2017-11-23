#version 410 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
	vec4 FragPosViewSpace;
    vec3 Normal;
    vec2 TexCoords;
	flat int numLights;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D layerShadowMaps[10];

uniform vec3 lightPositions[100];
uniform float linearAttn[100];
uniform float quadraticAttn[100];
uniform mat4 lightSpaceMatrix[100];
uniform vec3 lightColors[100];
uniform int assignedLayer[100];
uniform vec3 viewPos;

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
	vec4 FragPosLightSpace[100];
	vec3 ambient = 0.3 * color;
	vec3 lighting = vec3(0.0);
	lighting = ambient * color;
	float shadow;
	for(int i = 0; i< fs_in.numLights; i++)
	{
	    FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(fs_in.FragPos, 1.0);
		vec3 projCoords = FragPosLightSpace[i].xyz / FragPosLightSpace[i].w;

		if(projCoords.x >= -1.0 && projCoords.x <=1.0 && projCoords.y >= -1.0 && projCoords.y <=1.0 && 
			projCoords.z >= -1.0 && projCoords.z <=1.0)
		{
			vec3 lightColor = lightColors[i];
			vec3 lightPos = lightPositions[i];
			vec3 lightDir = normalize(lightPos - fs_in.FragPos);
			float distance = length(lightPos - fs_in.FragPos);

			float attenuation  = 1.0/(1.0 + linearAttn[i] * distance + quadraticAttn[i] * distance * distance);
			float diff = max(dot(lightDir, normal), 0.0);
			vec3 diffuse = diff * lightColor;
			vec3 viewDir = normalize(viewPos - fs_in.FragPos);
			vec3 reflectDir = reflect(-lightDir, normal);
			float spec = 0.0;
			vec3 halfwayDir = normalize(lightDir + viewDir);
			spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
			vec3 specular = spec * lightColor;    
			vec3 viewSpaceCoords = fs_in.FragPosViewSpace.xyz / fs_in.FragPosViewSpace.w;
			viewSpaceCoords = viewSpaceCoords * 0.5 + 0.5;
			shadow = texture(layerShadowMaps[assignedLayer[i]], viewSpaceCoords.xy).r;
			lighting += ((1.0 - shadow) * (diffuse + specular)) * color * attenuation;
			//lighting = vec3(shadow);
		}

	}
	    
    FragColor = vec4(lighting, 1.0);
	//FragColor = vec4(1.0f);
}