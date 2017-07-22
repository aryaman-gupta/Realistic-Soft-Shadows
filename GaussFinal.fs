#version 410 core

in vec4 IPCoords;
in vec4 FragPosViewSpace;
in vec3 inNormal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;
uniform sampler2D FinalShadowMap;
uniform sampler2D diffuseTexture;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
	vec3 projCoords = IPCoords.xyz / IPCoords.w;
	projCoords = projCoords * 0.5 + 0.5;			
	vec3 color = texture(diffuseTexture, vec2(1, 1)).rgb;
	//color = vec3(1, 0, 0);
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
	float shadowVal = texture(FinalShadowMap, projCoords.xy).r;
	lighting = (ambient + (1.0 - shadowVal) * (diffuse + specular)) * color;
	FragColor = vec4(lighting, 1.0);
	//FragColor = vec4(0);
}